#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rules_all:"suppress all MISRA rules for test")
#endif

#include <stdio.h>
#include <stdlib.h>
#include "arm_math.h"

#include "test_common.h"

#include "afe.h"
#include "afe_lib.h"
#include "uart.h"
#include "gpio.h"

#include <ADuCM350_device.h>

#if defined ( __ICCARM__ )  // IAR compiler...
/* Apply ADI MISRA Suppressions */
#define ASSERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"
#endif

/* Size of Tx and Rx buffers */
#define BUFFER_SIZE     32

static char RxBuffer[BUFFER_SIZE];
static char TxBuffer[BUFFER_SIZE];

ADI_UART_HANDLE     hUartDevice;


#define USE_UART_FOR_DATA  (1)

//#define FREQ (30000)

#define VPEAK (1200)

#define RCAL (1000)

//#define FCW                         ((uint32_t)(((uint64_t)FREQ << 26) / 16000000 + 0.5))
#define DAC_LSB_SIZE                (0.39072)
#define SINE_AMPLITUDE              ((uint16_t)((VPEAK * 40) / DAC_LSB_SIZE + 0.5))

#define DFT_RESULTS_OPEN_MAX_THR    (10)
#define DFT_RESULTS_OPEN_MIN_THR    (-10)

#define DFT_RESULTS_COUNT           (8)

#define FIXED32_LSB_SIZE            (625)
#define MSG_MAXLEN                  (50)

#define PRINT(s)                    test_print(s)

typedef union {
    int32_t     full;
    struct {
        uint8_t fpart:4;
        int32_t ipart:28;
    } parts;
} fixed32_t;



//prototipos
q15_t           arctan                  (q15_t imag, q15_t real);
fixed32_t       calculate_magnitude     (q31_t magnitude_rcal, q31_t magnitude_z);
fixed32_t       calculate_phase         (q15_t phase_rcal, q15_t phase_z);
void            convert_dft_results     (int16_t *dft_results, q15_t *dft_results_q15, q31_t *dft_results_q31);
void                    sprintf_fixed32         (char *out, fixed32_t in);
void                    print_MagnitudePhase    (char *text, fixed32_t magnitude, fixed32_t phase);
void                    test_print              (char *pBuffer);
ADI_UART_RESULT_TYPE    uart_Init               (void);
ADI_UART_RESULT_TYPE    uart_UnInit             (void);
extern int32_t          adi_initpinmux          (void);


uint32_t seq_afe_acmeas2wire[] = {
    0x001D0043,   /* Safety word: bits 31:16 = command count, bits 7:0 = CRC */
    0x84005818,   /* AFE_FIFO_CFG: DATA_FIFO_SOURCE_SEL = 10 */
    0x8A000034,   /* AFE_WG_CFG: TYPE_SEL = 10 */
    0x98000000,   /* AFE_WG_CFG: SINE_FCW = 0 (placeholder, user programmable) */
    0x9E000000,   /* AFE_WG_AMPLITUDE: SINE_AMPLITUDE = 0 (placeholder, user programmable) */
    0x88000F01,   /* AFE_DAC_CFG: DAC_ATTEN_EN = 1 */
    0xA0000002,   /* AFE_ADC_CFG: MUX_SEL = 00010, GAIN_OFFS_SEL = 00 */
    /* RCAL */
    0x86008811,   /* DMUX_STATE = 1, PMUX_STATE = 1, NMUX_STATE = 8, TMUX_STATE = 8 */
    0x00000640,   /* Wait 100us */
    0x80024EF0,   /* AFE_CFG: WAVEGEN_EN = 1 */
    0x00000C80,   /* Wait 200us */
    0x8002CFF0,   /* AFE_CFG: ADC_CONV_EN = 1, DFT_EN = 1 */
    0x00032340,   /* Wait 13ms */
    0x80024EF0,   /* AFE_CFG: ADC_CONV_EN = 0, DFT_EN = 0 */
    /* AFE3 - AFE4 */
    0x86003344,   /* DMUX_STATE = 3, PMUX_STATE = 3, NMUX_STATE = 4, TMUX_STATE = 4 */
    0x00000640,   /* Wait 100us */
    0x8002CFF0,   /* AFE_CFG: ADC_CONV_EN = 1, DFT_EN = 1 */
    0x00032340,   /* Wait 13ms */
    0x80024EF0,   /* AFE_CFG: ADC_CONV_EN = 0, DFT_EN = 0 */
    /* AFE4 - AFE5 */
    0x86004455,   /* DMUX_STATE = 4, PMUX_STATE = 4, NMUX_STATE = 5, TMUX_STATE = 5 */
    0x00000640,   /* Wait 100us */
    0x8002CFF0,   /* AFE_CFG: ADC_CONV_EN = 1, DFT_EN = 1 */
    0x00032340,   /* Wait 13ms */
    0x80024EF0,   /* AFE_CFG: ADC_CONV_EN = 0, DFT_EN = 0 */
    /* AFE3 - AFE5 */
    0x86005566,   /* DMUX_STATE = 3, PMUX_STATE = 3, NMUX_STATE = 5, TMUX_STATE = 5 */ //0x86003355,   /* DMUX_STATE = 3, PMUX_STATE = 3, NMUX_STATE = 5, TMUX_STATE = 5 */
    0x00000640,   /* Wait 100us */
    0x8002CFF0,   /* AFE_CFG: ADC_CONV_EN = 1, DFT_EN = 1 */
    0x00032340,   /* Wait 13ms */
    0x80020EF0,   /* AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, DFT_EN = 0 */

    0x82000002,   /* AFE_SEQ_CFG: SEQ_EN = 0 */
};

bool trataComando();
bool frequencySweep(void);
bool sweepFlag=false, commandFlag=true;

char cBuffer[32];

uint64_t startFrequency = 0, finalFrequency = 90000, sweep = 100, testFrequency=30000;

bool bStopFlag = true;

int main(void) {
    ADI_AFE_DEV_HANDLE  hDevice;
    int16_t             dft_results[DFT_RESULTS_COUNT];
    q15_t               dft_results_q15[DFT_RESULTS_COUNT];
    q31_t               dft_results_q31[DFT_RESULTS_COUNT];
    q31_t               magnitude[DFT_RESULTS_COUNT / 2];
    q15_t               phase[DFT_RESULTS_COUNT / 2];
    fixed32_t           magnitude_result[DFT_RESULTS_COUNT / 2 - 1];
    fixed32_t           phase_result[DFT_RESULTS_COUNT / 2 - 1];
    char                msg[MSG_MAXLEN];
    int32_t              i;
    uint64_t FREQ, FCW;


    //UART Things
    ADI_UART_RESULT_TYPE uartResult;
    ADI_UART_INIT_DATA   initData;
    ADI_UART_GENERIC_SETTINGS_TYPE  Settings;
    int16_t  rxSize;
    int16_t  txSize;





    SystemInit();
    //UART
    NVIC_SetPriorityGrouping(12);

    SystemTransitionClocks(ADI_SYS_CLOCK_TRIGGER_MEASUREMENT_ON);

    SetSystemClockDivider(ADI_SYS_CLOCK_UART, 2);

    test_Init();

    adi_initpinmux();

    if(ADI_UART_SUCCESS != uart_Init())
      FAIL("uart_Init");

    if(adi_AFE_Init(&hDevice))
      FAIL("adi_AFE_Init");

    if (adi_AFE_PowerUp(hDevice))
      FAIL("adi_AFE_PowerUp");

    if (adi_AFE_ExciteChanPowerUp(hDevice))
      FAIL("adi_AFE_ExciteChanPowerUp");

    if (adi_AFE_TiaChanCal(hDevice))
      FAIL("adi_AFE_TiaChanCal");

    if (adi_AFE_ExciteChanCalAtten(hDevice))
      FAIL("adi_AFE_ExciteChanCalAtten");


   //initialize uart
    /*initData.pRxBufferData = (uint8_t*)RxBuffer;
    initData.RxBufferSize = BUFFER_SIZE;
    initData.pTxBufferData = (uint8_t*)TxBuffer;
    initData.TxBufferSize = BUFFER_SIZE;*/

    /* Open UART driver */
    /*uartResult = adi_UART_Init(ADI_UART_DEVID_0, &hUartDevice, &initData);
    if (ADI_UART_SUCCESS != uartResult)
    {
        test_Fail("adi_UART_Init() failed");
    }

    Settings.BaudRate = ADI_UART_BAUD_115200;
    Settings.bBlockingMode = true;
    Settings.bInterruptMode = true;
    Settings.Parity = ADI_UART_PARITY_NONE;
    Settings.WordLength = ADI_UART_WLS_8;
    Settings.bDmaMode = false;*/

    /* config UART */
    /*uartResult =  adi_UART_SetGenericSettings(hUartDevice, &Settings);
    if (ADI_UART_SUCCESS != uartResult)
    {
        test_Fail("adi_UART_SetGenericSettings() failed");
    }*/

    /* enable UART */
    uartResult = adi_UART_Enable(hUartDevice, true);

    //ADI_INSTALL_HANDLER(hUartDevice, interrupt);

    if (ADI_UART_SUCCESS != uartResult)
    {
        test_Fail("adi_UART_Enable(true) failed");
    }

    while(true){
      FREQ = testFrequency;
      FCW = ((uint32_t)(((uint64_t)FREQ << 26) / 16000000 + 0.5));
      seq_afe_acmeas2wire[3] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_FCW, FCW);
      seq_afe_acmeas2wire[4] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_AMPLITUDE, SINE_AMPLITUDE);


      uartResult = adi_UART_Enable(hUartDevice, false);
      uartResult = adi_UART_SetBlockingMode(hUartDevice, true);
      uartResult = adi_UART_SetInterruptMode(hUartDevice, false);
      uartResult = adi_UART_Enable(hUartDevice, true);

      adi_AFE_EnableSoftwareCRC(hDevice, true);

      if (adi_AFE_RunSequence(hDevice, seq_afe_acmeas2wire, (uint16_t *)dft_results, DFT_RESULTS_COUNT))
        FAIL("Impedance Measurement");

      adi_AFE_EnableSoftwareCRC(hDevice, false);

        /* Print DFT complex results */
      sprintf(msg, "CC1:%6d, %6d|", dft_results[2], dft_results[3]);
      PRINT(msg);

      convert_dft_results(dft_results, dft_results_q15, dft_results_q31);
      arm_cmplx_mag_q31(dft_results_q31, magnitude, DFT_RESULTS_COUNT / 2);

      for (i = 0; i < DFT_RESULTS_COUNT / 2 - 1; i++)
        magnitude_result[i] = calculate_magnitude(magnitude[0], magnitude[i + 1]);

      phase[0] = arctan(dft_results[1], dft_results[0]);
      for (i = 0; i < DFT_RESULTS_COUNT / 2 - 1; i++){
          if (magnitude_result[i].full) {
              phase[i + 1]         = arctan(dft_results[2 * (i + 1) + 1], dft_results[2 * (i + 1)]);

              phase_result[i]      = calculate_phase(phase[0], phase[i + 1]);
          }
          else {
              phase[i + 1]         = 0;
              phase_result[i].full = 0;
          }
      }


      for(i = 0; i < 1000; i++);

        //PRINT("Final results (magnitude, phase):\r\n");
      print_MagnitudePhase("CH1:", magnitude_result[0], phase_result[0]);
      //print_MagnitudePhase("CH2|", magnitude_result[1], phase_result[1]);
     // print_MagnitudePhase("CH3|", magnitude_result[2], phase_result[2]);
      char mensagem[16];
      sprintf(mensagem, "FC:%d|", testFrequency);
      PRINT(mensagem);

      for(i = 0; i < 1000; i++);

     uartResult = adi_UART_Enable(hUartDevice, false);
     uartResult = adi_UART_SetBlockingMode(hUartDevice, false);
     uartResult = adi_UART_SetInterruptMode(hUartDevice, true);
     uartResult = adi_UART_Enable(hUartDevice, true);
     for(i = 0; i < 1000; i++);

      int contador = 0;

      rxSize = 1;
      txSize = 1;


      uartResult = adi_UART_BufRx(hUartDevice, RxBuffer, &rxSize);
      if (ADI_UART_SUCCESS != uartResult)
      {
        test_Fail("adi_UART_BufRx() failed");
      }
      for(i = 0; i < 1000; i++);

      adi_UART_BufFlush(hUartDevice);

      if(RxBuffer[0] > 0 || sweepFlag){
        uartResult = adi_UART_Enable(hUartDevice, false);
        uartResult = adi_UART_SetBlockingMode(hUartDevice, true);
        uartResult = adi_UART_SetInterruptMode(hUartDevice, false);
        uartResult = adi_UART_Enable(hUartDevice, true);
        if(!sweepFlag){
          bStopFlag = false;
          PRINT("ok|");
        }
        while(bStopFlag == false){
          rxSize = 1;
          txSize = 1;
          uartResult = adi_UART_BufRx(hUartDevice, RxBuffer, &rxSize);
          if (ADI_UART_SUCCESS != uartResult)
          {
            test_Fail("adi_UART_BufRx() failed");
          }
            /* transmit the character */
          cBuffer[contador] = RxBuffer[0];
          contador++;

          if(RxBuffer[0] == '#')
          {
              RxBuffer[0] = 0;
              bStopFlag = true;
              commandFlag=true;
              PRINT("ok|");
          }

        }

        if(commandFlag){
          sweepFlag = trataComando();
        }

        if(sweepFlag){
          commandFlag = false;
          FREQ = startFrequency;
          FCW = ((uint32_t)(((uint64_t)FREQ << 26) / 16000000 + 0.5));
          seq_afe_acmeas2wire[3] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_FCW, FCW);
          seq_afe_acmeas2wire[4] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_AMPLITUDE, SINE_AMPLITUDE);

          adi_AFE_EnableSoftwareCRC(hDevice, true);

          if (adi_AFE_RunSequence(hDevice, seq_afe_acmeas2wire, (uint16_t *)dft_results, DFT_RESULTS_COUNT))
            FAIL("Impedance Measurement");

          adi_AFE_EnableSoftwareCRC(hDevice, false);
          convert_dft_results(dft_results, dft_results_q15, dft_results_q31);
          arm_cmplx_mag_q31(dft_results_q31, magnitude, DFT_RESULTS_COUNT / 2);

          for (i = 0; i < DFT_RESULTS_COUNT / 2 - 1; i++)
            magnitude_result[i] = calculate_magnitude(magnitude[0], magnitude[i + 1]);

          phase[0] = arctan(dft_results[1], dft_results[0]);
          for (i = 0; i < DFT_RESULTS_COUNT / 2 - 1; i++){
              if (magnitude_result[i].full) {
                  phase[i + 1]         = arctan(dft_results[2 * (i + 1) + 1], dft_results[2 * (i + 1)]);

                  phase_result[i]      = calculate_phase(phase[0], phase[i + 1]);
              }
              else {
                  phase[i + 1]         = 0;
                  phase_result[i].full = 0;
              }
          }

            //PRINT("Final results (magnitude, phase):\r\n");
          print_MagnitudePhase("FS:", magnitude_result[0], phase_result[0]);
          char mensagem[16];
          sprintf(mensagem, "FF:%d|", startFrequency);
          PRINT(mensagem);



          startFrequency += sweep;

          if(startFrequency > finalFrequency)
            sweepFlag = false;
        }
        memset(cBuffer, NULL, sizeof(cBuffer));
        adi_UART_BufFlush(hUartDevice);
      }

      memset(cBuffer, NULL, sizeof(cBuffer));
      adi_UART_BufFlush(hUartDevice);

    }

     if (adi_AFE_PowerDown(hDevice))
       FAIL("adi_AFE_PowerDown");

    if (adi_AFE_UnInit(hDevice)) {
        FAIL("adi_AFE_UnInit");
    }

    adi_UART_UnInit(hUartDevice);

    PASS();
}



bool trataComando(){
  int8_t bufferSize;
  char commandBuffer[3];
  bool bufferFlag = false;

  int bufferIndex = 0;
  int count = 0;
  int delimiterIndex[10];
  char msg[3][20], debuggerMessage[32];
  int msgIndex = 0, conversionIndex = 0;

  int i, j;

  //memset(commandBuffer, NULL, sizeof(commandBuffer));
  bufferSize = strlen(cBuffer);



  if(bufferSize > 2){
    strncpy(commandBuffer, cBuffer, 2);
    commandBuffer[2] = '\0';
    bufferFlag = true;
  }

  if(bufferFlag){
    //PRINT("\n");
    if(strcmp(commandBuffer, "AF") == 0){
      while(cBuffer[bufferIndex] != '#'){
        if(cBuffer[bufferIndex] == '|'){
          delimiterIndex[count++] = bufferIndex;
        }
        bufferIndex++;
      }

      delimiterIndex[count] = bufferIndex;


      for(i = 0; i<count; i++){
        for(j = delimiterIndex[i]+1; j < delimiterIndex[i+1]; j++){
          msg[conversionIndex][msgIndex] = cBuffer[j];
          msgIndex++;
        }
        msg[conversionIndex][msgIndex] = '\0';
        if(conversionIndex == 0)
          testFrequency = atoi(msg[0]);
        conversionIndex++;
        msgIndex=0;
      }

      //sprintf(debuggerMessage, "test frequency: %d\n", testFrequency);
      //PRINT(debuggerMessage);

      return false;
    }



    if(strcmp(commandBuffer, "FS") == 0){
      //PRINT("\n");
      while(cBuffer[bufferIndex] != '#'){
        if(cBuffer[bufferIndex] == '|'){
          delimiterIndex[count++] = bufferIndex;
        }
        bufferIndex++;
      }

      delimiterIndex[count] = bufferIndex;

      conversionIndex = 0;
      for(i = 0; i<count; i++){
        for(j = delimiterIndex[i]+1; j < delimiterIndex[i+1]; j++){
          msg[conversionIndex][msgIndex] = cBuffer[j];
          msgIndex++;
        }
        msg[conversionIndex++][msgIndex] = '\0';
        msgIndex=0;
      }

      startFrequency = atoi(msg[0]);
      finalFrequency = atoi(msg[1]);
      sweep = atoi(msg[2]);

      /*sprintf(debuggerMessage, "start frequency: %d\n", startFrequency);
      PRINT(debuggerMessage);
      sprintf(debuggerMessage, "final frequency: %d\n", finalFrequency);
      PRINT(debuggerMessage);
      sprintf(debuggerMessage, "sweep frequency: %d\n", sweep);
      PRINT(debuggerMessage);*/

      return true;
    }

  }


  return false;
}

bool frequencySweep(void){

  return false;
}

const q15_t coeff[5] = {
    (q15_t)0x28BD,     /*  0.318253 */
    (q15_t)0x006D,     /*  0.003314 */
    (q15_t)0xEF3E,     /* -0.130908 */
    (q15_t)0x08C6,     /*  0.068542 */
    (q15_t)0xFED4,     /* -0.009159 */
};

q15_t arctan(q15_t imag, q15_t real) {
    q15_t       t;
    q15_t       out;
    uint8_t     rotation; /* Clockwise, multiples of PI/4 */
    int8_t      i;

    if ((q15_t)0 == imag) {
        /* Check the sign*/
        if (real & (q15_t)0x8000) {
            /* Negative, return -PI */
            return (q15_t)0x8000;
        }
        else {
            return (q15_t)0;
        }
    }
    else {

        rotation = 0;
        /* Rotate the vector until it's placed in the first octant (0..PI/4) */
        if (imag < 0) {
            imag      = -imag;
            real      = -real;
            rotation += 4;
        }
        if (real <= 0) {
            /* Using 't' as temporary storage before its normal usage */
            t         = real;
            real      = imag;
            imag      = -t;
            rotation += 2;
        }
        if (real <= imag) {
            /* The addition below may overflow, drop 1 LSB precision if needed. */
            /* The subtraction cannot underflow.                                */
            t = real + imag;
            if (t < 0) {
                /* Overflow */
                t         = imag - real;
                real      = (q15_t)(((q31_t)real + (q31_t)imag) >> 1);
                imag      = t >> 1;
            }
            else {
                t         = imag - real;
                real      = (real + imag);
                imag      = t;
            }
            rotation += 1;
        }

        /* Calculate tangent value */
        t = (q15_t)((q31_t)(imag << 15) / real);

        out = (q15_t)0;

        for (i = 4; i >=0; i--) {
            out += coeff[i];
            arm_mult_q15(&out, &t, &out, 1);
        }

        /* Rotate back to original position, in multiples of pi/4 */
        /* We're using 1.15 representation, scaled by pi, so pi/4 = 0x2000 */
        out += (rotation << 13);

        return out;
    }
}

void convert_dft_results(int16_t *dft_results, q15_t *dft_results_q15, q31_t *dft_results_q31) {
    int8_t      i;

    for (i = 0; i < (DFT_RESULTS_COUNT / 2); i++) {
        if ((dft_results[i] < DFT_RESULTS_OPEN_MAX_THR) &&
            (dft_results[i] > DFT_RESULTS_OPEN_MIN_THR) &&               /* real part */
            (dft_results[2 * i + 1] < DFT_RESULTS_OPEN_MAX_THR) &&
            (dft_results[2 * i + 1] > DFT_RESULTS_OPEN_MIN_THR)) {       /* imaginary part */

            /* Open circuit, force both real and imaginary parts to 0 */
            dft_results[i]           = 0;
            dft_results[2 * i + 1]   = 0;
        }
    }

    /*  Convert to 1.15 format */
    for (i = 0; i < DFT_RESULTS_COUNT; i++) {
        dft_results_q15[i] = (q15_t)dft_results[i];
    }

    /*  Convert to 1.31 format */
    arm_q15_to_q31(dft_results_q15, dft_results_q31, DFT_RESULTS_COUNT);

}

fixed32_t calculate_magnitude(q31_t magnitude_rcal, q31_t magnitude_z) {
    q63_t       magnitude;
    fixed32_t   out;

    magnitude = (q63_t)0;
    if ((q63_t)0 != magnitude_z) {
        magnitude = (q63_t)magnitude_rcal * (q63_t)RCAL;
        /* Shift up for additional precision and rounding */
        magnitude = (magnitude << 5) / (q63_t)magnitude_z;
        /* Rounding */
        magnitude = (magnitude + 1) >> 1;
    }

    /* Saturate if needed */
    if (magnitude &  0xFFFFFFFF00000000) {
        /* Cannot be negative */
        out.full = 0x7FFFFFFF;
    }
    else {
        out.full = magnitude & 0xFFFFFFFF;
    }

    return out;
}

fixed32_t calculate_phase(q15_t phase_rcal, q15_t phase_z) {
    q63_t       phase;
    fixed32_t   out;

    /* Multiply by 180 to convert to degrees */
    phase = ((q63_t)(phase_z - phase_rcal) * (q63_t)180);
    /* Round and convert to fixed32_t */
    out.full = ((phase + (q63_t)0x400) >> 11) & 0xFFFFFFFF;

    return out;
}

void sprintf_fixed32(char *out, fixed32_t in) {
    fixed32_t   tmp;

    if (in.full < 0) {
        tmp.parts.fpart = (16 - in.parts.fpart) & 0x0F;
        tmp.parts.ipart = in.parts.ipart;
        if (0 != in.parts.fpart) {
            tmp.parts.ipart++;
        }
        if (0 == tmp.parts.ipart) {
            sprintf(out, "      -0.%04d", tmp.parts.fpart * FIXED32_LSB_SIZE);
        }
        else {
            sprintf(out, "%8d.%04d", tmp.parts.ipart, tmp.parts.fpart * FIXED32_LSB_SIZE);
        }
    }
    else {
        sprintf(out, "%8d.%04d", in.parts.ipart, in.parts.fpart * FIXED32_LSB_SIZE);
    }

}

void print_MagnitudePhase(char *text, fixed32_t magnitude, fixed32_t phase) {
    char                msg[MSG_MAXLEN];
    char                tmp[MSG_MAXLEN];

    sprintf(msg, "%s", text);
    /* Magnitude */
    sprintf_fixed32(tmp, magnitude);
    strcat(msg, tmp);
    strcat(msg, ",");
    /* Phase */
    sprintf_fixed32(tmp, phase);
    strcat(msg, tmp);
    strcat(msg, "|");
    //strcat(msg, ")\r\n");

    PRINT(msg);
}

void test_print (char *pBuffer) {
#if (1 == USE_UART_FOR_DATA)
    int16_t size;
    /* Print to UART */
    size = strlen(pBuffer);
    adi_UART_BufTx(hUartDevice, pBuffer, &size);

#elif (0 == USE_UART_FOR_DATA)
    /* Print  to console */
    printf(pBuffer);

#endif /* USE_UART_FOR_DATA */
}

/* Initialize the UART, set the baud rate and enable */
ADI_UART_RESULT_TYPE uart_Init (void) {
    ADI_UART_RESULT_TYPE    result = ADI_UART_SUCCESS;

    /* Open UART in blocking, non-intrrpt mode by supplying no internal buffs */
    if (ADI_UART_SUCCESS != (result = adi_UART_Init(ADI_UART_DEVID_0, &hUartDevice, NULL)))
    {
        return result;
    }

    /* Set UART baud rate to 115200 */
    if (ADI_UART_SUCCESS != (result = adi_UART_SetBaudRate(hUartDevice, ADI_UART_BAUD_115200)))
    {
        return result;
    }

    /* Enable UART */
    if (ADI_UART_SUCCESS != (result = adi_UART_Enable(hUartDevice,true)))
    {
        return result;
    }

    return result;
}

/* Uninitialize the UART */
ADI_UART_RESULT_TYPE uart_UnInit (void) {
    ADI_UART_RESULT_TYPE    result = ADI_UART_SUCCESS;

    /* Uninitialize the UART API */
    if (ADI_UART_SUCCESS != (result = adi_UART_UnInit(hUartDevice)))
    {
        return result;
    }

    return result;
}
