<template>
  <div id="graphCard">
    <highcharts :options="chartOptions"></highcharts>
  </div>
</template>

<script>
import io from 'socket.io-client'
import {EventBus} from "../main.js"

var output={
  
};

var comando, updateFlag = false;

export default {
  name: 'graphCard',
  data: function(){
    return{
      socket: io("http://localhost:3000"),
      chartOptions: createChartOptions,
      magnitudeStats: [],
      faseStats: []

    }
  },
  timers: {
    listen: {time: 200, autostart:true, repeat: true},
    updateChart: {time: 5000, autostart:true, repeat:true}
  },
  mounted(){
    EventBus.$on('comando-tela', data => {
        this.socket.emit('abrir-comando', "ok");
        setTimeout(() => {
            this.socket.emit('comando-front', data);
            this.socket.on('abrir-comandofb', datafb => {
                EventBus.$emit('comando-telafb', datafb);
            }
        )}, 200);
    })
  },

  methods: {
    listen: function(){
      //console.log(this.chartOptions);
      this.socket.emit('leitura?', "ok");
      this.socket.on('leitura', dados => {
        output = dados;
        console.log(output);
      })
      if(!updateFlag){
        this.magnitudeStats.push(parseFloat(output.creal));
        this.faseStats.push(parseFloat(output.cim));
      }

      EventBus.$emit('data', output);
    },
    updateChart: function(){
      updateFlag = true;
      var statLoop = 0;
      var auxMagnitude = 0;
      var auxFase = 0

      while(this.magnitudeStats.length > 0 && this.faseStats.length > 0){
        auxMagnitude += this.magnitudeStats.pop();
        auxFase += this.faseStats.pop();

        statLoop++;
      }
      
      auxMagnitude /= (statLoop-1);
      auxFase /= (statLoop-1)

      this.chartOptions.series[0].data.push([Date.now() - (1000*3600*3), (auxMagnitude)]);
      this.chartOptions.series[1].data.push([Date.now() - (1000*3600*3), auxFase]);

      updateFlag = false;
    }
  }
}

var createChartOptions = {
    chart: {
        type: 'spline',
        scrollablePlotArea: {
            minWidth: 600,
            scrollPositionX: 1
        }
    },
    title: {
        text: 'Gráfico de Impedancia',
        align: 'center'
    },
    subtitle:{
        text: 'Processo de Cura do Latex',
        align: 'center'
    },
    xAxis: {
        type: 'datetime',
        labels: {
            overflow: 'justify'
        }
    },
    yAxis: {
        title: {
            text: 'Magnitude [Ω]'
        },
        minorGridLineWidth: 0,
        gridLineWidth: 0,
        alternateGridColor: null,
        
    },
    tooltip: {
        valueSuffix: 'Ω'
    },
    plotOptions: {
        spline: {
            lineWidth: 4,
            states: {
                hover: {
                    lineWidth: 5
                }
            },
            marker: {
                enabled: false
            },
            //pointInterval: 1000, // one hour
            //pointStart: Date.now()
        }
    },
    series: [{
        name: 'Magnitude',
        data: []
    },{
        name: 'Fase',
        data: []
    }],
    navigation: {
        menuItemStyle: {
            fontSize: '10px'
        }
    }
}
</script>

<style scoped>
div{
  width: 100%;
  box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2);
  transition: 0.3s;
  border-radius: 0.6em;
}

div:hover{
  box-shadow: 0 8px 16px 0 rgba(0, 0, 0, 0.2);
}
</style>
