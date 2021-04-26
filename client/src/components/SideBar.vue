<template>
<div class="sidebar">
  <nav class="navbar-vertical fixed-left navbar-expand-md navbar-light bg-white" id="sidenav-main">
    <div id="header-div">
      <div id="sidebar-header">
        <p id="p-brand"><img id="logo-ntc" src="./img/logo_ntc-80.png"/>Técnologia em Compositos</p>
      </div>
      <hr id="divider">
      <ul class="navbar-nav" id="menu">
          <li class="nav-item active" id="item-dash" >
            <a class=" nav-link active " id="link-dash" href=" ./index.html"> <i id="icon-dash" class="mr-3 fas fa-tv"></i> Dashboard
            </a>
          </li>
          <li class="nav-item active" id="item-tga" >
            <a class=" nav-link active " id="link-tga" href="#"> <i id="icon-tga" class="mr-3 fas fa-chart-area"></i> Analise TGA
            </a>
          </li>

          <li class="nav-item active" id="item-dsc" >
            <a class=" nav-link active " id="link-dsc" href="#"> <i id="icon-dsc" class="mr-3 fas fa-chart-area"></i> Analise DSC
            </a>
          </li>

        </ul>
    </div>
  </nav>
</div>

</template> 

<script>
export default{

}
</script>

<style scoped>


.sidebar{
  margin: 0;
  font-family: Open Sans, sans-serif;
  font-size: 1rem;
  font-weight: 400;
  line-height: 1.5;
  color: #525f7f;
  background-color: #f8f9fe;
  display: flex;
  height: 100vh;
}

#sidenav-main{
    width: 20vw;  
    box-shadow: 0 8px 0px 0 rgba(0, 0, 0, 0.2) !important;
    transition: 0.3s;
    border-width: 0 0 1px 0;
    border-style: solid;
    display: flex;
    justify-content: center;    
}

#sidenav-main:hover{
  box-shadow: 0 16px 0px 0 rgba(0, 0, 0, 0.2);
}

#header-div{
   display: inline-block;   
}

#sidebar-header{
  margin-top: 20px;
  margin-left: 5px;
  margin-bottom: 20px;
}

#p-brand{
  font-size: 1rem;
}
#logo-ntc{
  max-width: 13%;
  height: auto;

  margin-right: 10px;
}

#divider{
  margin: 10px;
}

#menu{
  margin-top: 50px;
  justify-content: left;
  display: block;
}

#item-dash,
#item-dsc, 
#item-tga{
  color: #8898aa !important;        
}


#item-dash.active > a{
  color: #5e72e4;
}

#item-dash:hover,
#item-tga:hover,
#item-dsc:hover {
  color: #525f7f !important;
}


#link-dash,
#link-tga,
#link-dsc {
  color: #525f7f;
  vertical-align: middle;
  font-size: 1.1rem;
}

#icon-dash,
#icon-tga,
#icon-dsc {
  margin-left: 10px;
  padding-right: 5px;
  vertical-align: middle;
}

#link-dash:hover,
#link-tga:hover,
#link-dsc:hover {
  color: #5e72e4;
}



</style>