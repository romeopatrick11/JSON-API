/*
Copyright 2016 Nymi Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

console.log("buzz (aka notify) sample");

// open a websocket to the napi service
var napiServer = new WebSocket("wss://127.0.0.1:11000/napi");

napiServer.onopen = function () {
  var req = {
    op: "info",                         // To request info, the op is 'info' and the subop is 'get'
    subop: "get",
    devices: true,                      // we want information about any nymi bands in the vicinity
    exchange: "exchange-" + Date.now()  // this is a unique value to match a response to a request
  };

  napiServer.send(JSON.stringify(req));
}

napiServer.onmessage = function (event) {
  var authenticated = JSON.parse(event.data).provisionsPresent;
  if(authenticated === undefined){
    console.log("There are no authenticated or identified provisions.");
  }
  else{
    console.log("authenticated or identified provisions:", authenticated);
    var n = authenticated.length;

    napiServer.onmessage = napiServer.waitForOutcomes;

    for (var i = 0; i < n; i++) {
      var req = {
        op: "notify",
        subop: "run",
        exchange: "exchange-sign-" + authenticated[i] + "-" + Date.now(),
        pid: authenticated[i],
        notification: (i % 2) ? "positive" : "negative" // can be true and false as well
      }
      napiServer.send(JSON.stringify(req));
      console.log(i, "notifying:", authenticated[i], req.notification)
    }
  }
}

napiServer.waitForOutcomes = function (event) {
  var outcome = JSON.parse(event.data);

  if (("notify" == outcome.op) && ("run" == outcome.subop)) {
    if(outcome.successful){
      console.log("pid:", outcome.pid, "received a", outcome.notification, "notification")
    }
    else{
      console.log("FAILED notification:", outcome)
    }
  }
  else {
    console.log("ignoring message:", outcome);
  }
}


