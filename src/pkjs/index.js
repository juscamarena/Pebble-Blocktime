// default values
var price_usd = 0;
var height = 0;
var dictionary = {
  "KEY_PRICE": price_usd,
  "KEY_HEIGHT": height
};
  var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function apiSuccess(pos) {
  // API URL
  var url_block = "https://blockchain.info/latestblock";
  // Send request to blockchain.info
  xhrRequest(url_block, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with latest block #
      var json = JSON.parse(responseText);
      height = json.height;
      dictionary.KEY_HEIGHT = height;  
      console.log("Height: " + height);
    }  
  );
  // API URL
  var url = "https://api.bitcoinaverage.com/ticker/global/USD/";

  // Send request to bitcoinaverage
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with price info
      var json = JSON.parse(responseText);
      // Price of bitcoin
      price_usd = json.last; //last price USD
      console.log("Price is " + price_usd);
      dictionary.KEY_PRICE = price_usd;
      if (height === 0) {
        apiSuccess();
      }else {
          // Send to Pebble
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log("Bitcoin info sent to Pebble successfully!");
          },
          function(e) {
            console.log("Error sending bitcoin info to Pebble!");
          }
        );
      }
    }      
  );     
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
    // Get API data
    apiSuccess();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    apiSuccess();
  }                     
);
