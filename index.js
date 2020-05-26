var express=require("express");
var mqtt = require("mqtt");
var mysql = require("mysql");

var app= express();
app.set("view engine","ejs");

//mysql
const pool = mysql.createPool({
   host:"localhost",
   database:"proyek_embed",
   user:"root",
   password:""});
function getConnection() {
   return new Promise(function(resolve,reject){
      pool.getConnection(function(err,conn){
         if(err){
            reject(err);}
         else{
            resolve(conn);}});});}
function executeQuery(conn, query){
   return new Promise(function (resolve,reject) {
      conn.query(query, function (err,result) {
         if(err){reject(err);}
         else{
            resolve(result);}})});}
//
//init

var conn = null;
app.use(express.urlencoded({extended:true}));
app.listen(3000,async function(){
   conn = await getConnection();
   console.log("Listening to port 3000");});
//
//mqtt
var settings = {
    clientId: "MQTTJSS",
    keepalive: 1000,
    clean: false,
    reconnectPeriod: 100
}
var client  = mqtt.connect('mqtt://192.168.1.2',settings);
client.on("connect",function(){
   client.subscribe("proyek/kirim",function(err){
      console.log("mqtt connected");
   })
});

client.on("message",async function(topic,message){
   if(topic=="proyek/kirim"){
      var results=await executeQuery(conn,`insert into logging values('${message}')`);
      console.log("ada message baru: "+message);
   }
});
//endpoints
app.get("/getEntry",async function(req,res){
   var results = await executeQuery(conn,`select concat(concat(concat(substring(timestamp,1,5)," "),"  "),substring(timestamp,12,8)) as "hasil" from logging order by 1 desc`)
   client.publish("proyek/terima",results[0].hasil);
   console.log("satu entry: "+results[0].hasil);
   res.send(results[0].hasil);
});
app.get("/semuaData",async function (req,res) {
   var tanggal = req.query.tgl;
   console.log(tanggal);
   console.log("Load tabel");
   var results =null;
   if(tanggal == ""){
      console.log("iyes");
       results = await executeQuery(conn,`select * from logging order by 1  desc limit 20`);
   }
   else{
      console.log(`select * from logging where substring(timestamp,1,10) = '${tanggal}' order by 1 desc limit 20`);
      results = await executeQuery(conn,`select * from logging where substring(timestamp,1,10) = '${tanggal}' order by 1 desc limit 20`);
   }
   if(results==""){
      res.status(200).send("");
   }
   else{
      var temp=[];
      results.forEach(element => {
         var e = element.timestamp.split(";");
         temp.push({"tanggal":e[0],"waktu":e[1]})
      });
      res.status(200).send(temp);
  }
})
app.get("/showData",function(req,res){
   res.render("pages/stat");
})
app.get("/",function(req,res){
   res.redirect(302, "/showData");
})