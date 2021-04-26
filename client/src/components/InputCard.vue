<template>
<div style="width: 15rem;">
    
<div class="card card-stats mb-4 mb-lg-0">
    <div class="card-body">
        <div class="row">
            <div class="col">
                <h5 class="card-title text-uppercase text-muted mb-2">Comando</h5>
                <input v-model="comando" v-on:keyup.enter="trataComando" type="text" id="entrada" placeholder=" insira um comando">
            </div>
        </div>
        <p class="mt-2 mb-0 text-muted text-sm">
            <span v-if="success" class="text-success mr-2"><i class="fas fa-check"></i> Comando Recebido</span>
            <span v-if="!success" class="text-unsuccess mr-2"><i class="fas fa-times"></i> Comando Não Recebido</span>
        </p>
    </div>
</div>


</div>


</template>

<script>
import { EventBus } from '../main';
export default {
    data(){
        return{
            comando: [],
            success: null
        }
    },
    methods: {
        trataComando(){
            var temp;
            
            temp = this.comando.indexOf('#');

            if(temp > 0){
                
                EventBus.$emit('comando-tela', this.comando);
                EventBus.$on('comando-telafb', data => {
                  setTimeout(() => {
                    if(data == "ok"){
                      this.success = true;
                      setTimeout(() => {
                        this.success = false;
                      }, 3000);
                    }
                    else
                      this.sucess = false;
                  }, 250)
                })
            }
            else{
                this.success = false;
            }
        }
    }
}
</script>

<style scoped>

body{
  font-family: sans-serif;
  line-height: 1.15;
  -webkit-text-size-adjust: 100%;
  -ms-text-size-adjust: 100%;
  -ms-overflow-style: scrollbar;
  -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
}

#entrada{
    border-radius: 4px;
}

#entrada::placeholder{
    color: #8898aa;
    opacity: 0.3;
}

.row {
  display: flex;
  flex-wrap: wrap;
  margin-right: -15px;
  margin-left: -20px;
}

.col {
  flex-basis: 0;
  flex-grow: 1;
  max-width: 100%;
}

.icon i{
    font-size: 1.5rem;
}

.icon-shape {
  padding: 12px;
  text-align: center;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
}

.rounded-circle {
  border-radius: 50% !important;
}

.shadow{
    box-shadow: 0 0 2rem 0 rgba(136, 152, 170, 0.15) !important;
}

.card {
  position: relative;
  display: flex;
  flex-direction: column;
  min-width: 0;
  word-wrap: break-word;
  background-color: #fff;
  background-clip: border-box;
  border: 1px solid rgba(0, 0, 0, 0.05);
  border-radius: 0.375rem;

  box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2);
  transition: 0.3s;
}
.card:hover{
  box-shadow: 0 8px 16px 0 rgba(0, 0, 0, 0.2);
}

.card-stats .card-body {
  padding: 1rem 1.5rem;
}

.card-body {
  flex: 1 1 auto;
  padding: 1.5rem;
}

.card-title {
  margin-bottom: 1rem;
}

.h2 {
  margin-bottom: 0.5rem;
  font-family: inherit;
  font-weight: 600;
  line-height: 1.5;
  color: #32325d;
}

h2,
.h2 {
  font-size: 1.25rem;
}

h5 {
  font-size: 0.8125rem;
}

.text-uppercase {
  text-transform: uppercase !important;
}

.text-muted {
  color: #8898aa !important;
}

.font-weight-bold {
  font-weight: 600 !important;
}

.col-auto{
  flex: 0 0 auto;
  width: auto;
  max-width: none;
}

.col, .col-auto {
  position: relative;
  min-height: 1px;
  padding-right: 15px;
  padding-left: 15px;
}

.text-sm {
  font-size: 0.875rem !important;
}

.text-success {
  color: #2dce89 !important;
}

.text-unsuccess {
  color: red !important;
}

.bg-danger {
  background-color: #5e72e4 !important;
}

</style>