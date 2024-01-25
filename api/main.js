const express = require('express');
var fs     = require('fs');
var crypto = require('crypto');
var path   = require('path');
const users=[{"UID": "u1123", "username": "User1", "token": "token1"}]
const app = express ();
const tokens = ["token1"];
app.use(express.json());
const PORT = process.env.PORT || 5499;
var ActiveLinks =[

]
var Ads = [
  {"id": 1234,
  "name": "Sponsor1",
  "duration": "0:30"
},
{
  "id": 1235,
  "name": "Sponsor2",
  "duration": "0:46"
},
{
  "id": 1236,
  "name": "Sponsor2",
  "duration": "0:46"
}
]
var filelocations = [
  {"id": 1234,
"location": "./Sponsor1.mp4"},
{"id": 1235,
"location": "./Sponsor2.mp4"},
{"id": 1236,
"location": "./Sponsor2.mp4"}
]



const getAdfromID = (id) => {
return Ads.filter(it => it.id == id)[0];
}

const getLocationfromID = (id) => {
  return filelocations.filter(it => it.id == id)[0];
  }

  const getActiveLinkfromID = (id) => {
    return ActiveLinks.filter(it => it.dlID == id)[0];
  }
const getUserfromAuth = (token) => {
  return users.filter(it => it.token == token)[0];
  }

app.listen(PORT, () => {
    console.log("Server Listening on PORT:", PORT);
  });


  app.get("/getAds", (request, response) =>{
 console.log("/getAds got");

    if(checkrequest(request) != 200){
      response.sendStatus(checkrequest(request));
      return
    }


    response.send(Ads);

  }
  );


  app.get("/prepareAd", (request,response)=>
  {
    console.log(request);
    console.log(request.headers);
    console.log(request.headers["authorization"]);
    console.log(request.query["adID"]);

    if(checkrequest(request) != 200){
      response.sendStatus(checkrequest(request));
      return
    }

    console.log(getAdfromID(request.query["adID"]));
    var user = getUserfromAuth(request.headers["authorization"]);
    var id= createDownload();
    ActiveLinks.push({"user" : user.username, "dlID": id, "adID": request.query["adID"]})
    console.log("Activelinks:");
    console.log(ActiveLinks);
    response.send(id);


  })


  app.get("/loadAd", (request,response)=>
  {
    //TODO check if UID in active sessions
    var dlid = request.query["dlID"]
    var adID = request.query["adID"]
    var status = checkdlotp(dlid);
    console.log(status);
    if( status != 200){
      response.sendStatus(status);
      return;
    }
    var locationInfo = getLocationfromID(adID);
    response.download(locationInfo.location);

    console.log(dlid); 
    ActiveLinks.splice(ActiveLinks.find(it => it[1] == dlid));
    console.log(ActiveLinks);
  });

  function checkdlotp(id){
    console.log(id)
    if(typeof(id) === typeof(undefined))
    {
      return 403;
    }
    var possibleLink = getActiveLinkfromID(id);
    console.log(possibleLink);
    if(typeof(possibleLink) == typeof(undefined) || possibleLink["dlID"] != id ){
      return 401;
    }
    return 200;
  }

  function checkrequest(request){

    if(typeof(request.headers.authorization) ==typeof(undefined))
    {
      console.log("NO TOKEN");
      return 401;
    }
    else if(!(tokens.includes(request.headers.authorization))){
      console.log("FALSE TOKEN");
      return 403;
    }

    return 200;

  }

function createDownload() {
  var ID  = crypto.createHash('md5').update(Math.random().toString()).digest('hex');
  return ID;
}
