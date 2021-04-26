<template>
  <div id="fsGraphCard">
    <highcharts :options="chartOptions"></highcharts>
  </div>
</template>

<script>
import {EventBus} from "../main.js"

var output={
  
};

var contador = 0;

export default {
  name: 'fsGraphCard',
  data: function(){
    return{
      chartOptions: createChartOptions
    }
  },
  mounted(){
      EventBus.$on('data', data => {
          output = data;
          this.updateChart();
      })
  },
  methods: {
    updateChart: function(){
        if(output.csFlag){
            this.chartOptions.series[0].data.push([output.frequencySweep, parseFloat(output.freal)/1000]);
        }
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
        text: 'Gráfico de Incremento de Frequência',
        align: 'center'
    },
    subtitle:{
        text: 'Processo de Cura do Latex',
        align: 'center'
    },
    xAxis: {
        labels: {
            overflow: 'justify'
        }
    },
    yAxis: {
        title: {
            text: 'Impedancia k[Ω]'
        },
        minorGridLineWidth: 0,
        gridLineWidth: 0,
        alternateGridColor: null,
        
    },
    tooltip: {
        valueSuffix: 'kΩ'
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
            //pointStart: 0
        }
    },
    series: [{
        name: 'Magnitude',
        data: [[]]
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
