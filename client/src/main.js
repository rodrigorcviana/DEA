import Vue from 'vue'
import App from './App.vue'
import VueTimers from 'vue-timers'
import BootstrapVue from 'bootstrap-vue'

import HighchartsVue from 'highcharts-vue';
import Highcharts from 'highcharts';
import DataInit from 'highcharts/modules/data';
import SeriesInit from 'highcharts/modules/series-label';
import ExportingInit from 'highcharts/modules/exporting';
import ExportDataInit from 'highcharts/modules/export-data';

// app.js
import 'bootstrap/dist/css/bootstrap.css'
import 'bootstrap-vue/dist/bootstrap-vue.css'

export const EventBus = new Vue();

Vue.config.productionTip = false

Vue.use(VueTimers)

Vue.use(BootstrapVue)

Vue.use(HighchartsVue);

DataInit(Highcharts);
SeriesInit(Highcharts);
ExportingInit(Highcharts);
ExportDataInit(Highcharts);


new Vue({
  render: h => h(App),
}).$mount('#app')
