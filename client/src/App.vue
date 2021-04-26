<template>
  <div id="app">
    <div id="side-div">
      <side-bar/>
    </div>
    <div id="content">
      <div id="divider-main">
        <p>Dashboard</p>
        <hr>
      </div>
      <div id="data-content">
        <div id="cards">
          <impedancia-card id="imp-card"/>
          <fase-card id="fase-card"/>
          <frequencia-card id="fs-card"/>
          <input-card id="i-card"/>
        </div>
        <div id="graphs">
          <graph-card id="imp-graph"/>
          <fsgraph-card/>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import io from 'socket.io-client'
import GraphCard from './components/GraphCard'
import FsGraphCard from './components/FsGraphCard'
import SideBar from './components/SideBar'
import ImpedanciaCard from './components/ImpedanciaCard'
import FaseCard from './components/FaseCard'
import FrequenciaCard from './components/FrequenciaCard'
import InputCard from './components/InputCard'

export default {
  name: 'app',
  data: function(){
    return{
    }
  },
  components: {
    'graph-card': GraphCard,
    'fsgraph-card': FsGraphCard,
    'side-bar': SideBar,
    'impedancia-card': ImpedanciaCard,
    'frequencia-card': FrequenciaCard,
    'input-card': InputCard,
    'fase-card': FaseCard
  }
}
</script>

<style>
#app{
  display: inline-flex;
  min-width: 100%;
  
}
#side-div{
  display: block;
}

#content{
  background-color: #f5f5f5;
}

#divider-main{
  display: inline-block;
  padding-top: 20px;
  width: 100%;
}
#divider-main p{
  margin-left: 20px;
  font-size: 1.2rem;
  color: #8898aa;
}

#data-content{
  display: inline-block;
  justify-content: center;
  width: 100%;
}

#cards{
  display: flex;
  padding-top: 20px;
  padding-left: 55px;
}

#graphs{
  padding: 20px;
  flex-grow: 1;
  width: 100%
}

#imp-card{
  margin-left: -35px;
}
#fase-card{
  margin-left: 20px;
}
#fs-card{
  margin-left: 20px;
}
#i-card{
  margin-left: 20px;
}

#imp-graph{
  margin-bottom: 50px;
}
</style>
