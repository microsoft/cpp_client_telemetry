var seq = 0;
var allEvents = "";
var eventNames = [
  'Microsoft.WebBrowser.Protobuf.UKM.Stats.LogComplete',
  'Microsoft.WebBrowser.Protobuf.UKM.Aggregates',
  'Microsoft.WebBrowser.Protobuf.UKM.Report',
  'Microsoft.WebBrowser.Protobuf.UKM.Stats.LogStart',
  'Microsoft.WebBrowser.Mats.TransactionMicrosoftEdgeMac',
  'Microsoft.WebBrowser.Mats.ActionMicrosoftEdgeMac'
];

function mockEvent()
{
  var i = Math.floor(Math.random() * 6);
  var d = new Date();
  seq++;
  result ='<div class="event" data-index="'+seq+'" role="option">';
  result+='<div class="event-name">'+eventNames[i]+'</div>';
  result+='<div class="event-time">'+d.toISOString()+'</div>';
  return result;
};

window.cr = {

  sendWithPromise: function(methodName, var_args) {
    seq++;
    const args = Array.prototype.slice.call(arguments, 1);
    const id = methodName + '_' + seq;
    console.log(id, args);

    var promise = new Promise( (setResult) => {
      var result = "";
      switch (args[0])
      {
        /* settings */
        case "1":
          result = JSON.stringify({
            'should_use_utc': false,
            'not_registered': false,
            'is_uma_enabled': true,
          });
          setResult(result);
          break;
        /* event-list */
        case "2": {
            allEvents += mockEvent();
            setResult(allEvents);
/*
            var $ = jQuery.noConflict();
            var jqxhr = $.get('/events.txt', function(data) {
                setResult(data);
            });
*/
          break;
        }
        /* count-new-events */
        case "3":
          result = 100;
          setResult(result);
          break;
        /* payload-view */
        case "4": {
            var $ = jQuery.noConflict();
            $.get('/events/'+args[1]+'.json',
              function(data) {
                setResult(data);
              }).done(function() {
                // Nothing
              }).fail(function(data, textStatus, xhr) {
                //This shows status code eg. 403
                console.log("error", data.status);
                setResult(JSON.stringify(data));
              }).always(function() {
                // Nothing
              });
          break;
        }
      }
    });

    return promise;
  }
};