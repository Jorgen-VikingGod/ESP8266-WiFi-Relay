
function getAll() {
  $('#loadModal').modal('show');
  $.get(urlBase + 'all', function(data) {
    $('#loadModal').modal('hide');
    $('#status').html('Connecting...');
    $('#relay1Name').text(data.relay1.name);
    $('#relay2Name').text(data.relay2.name);
    $('#relay3Name').text(data.relay3.name);
    $('#relay4Name').text(data.relay4.name);
    $('#relay5Name').text(data.relay5.name);
    $('#relay6Name').text(data.relay6.name);
    $('#relay7Name').text(data.relay7.name);
    $('#relay8Name').text(data.relay8.name);
    updateToggleButton('relay1', data.relay1.state);
    updateToggleButton('relay2', data.relay2.state);
    updateToggleButton('relay3', data.relay3.state);
    updateToggleButton('relay4', data.relay4.state);
    updateToggleButton('relay5', data.relay5.state);
    updateToggleButton('relay6', data.relay6.state);
    updateToggleButton('relay7', data.relay7.state);
    updateToggleButton('relay8', data.relay8.state);
    $('#status').html('Ready');
  });
}

function getCurrent() {
  $('#loadModal').modal('show');
  $.get(urlBase + 'current', function(data) {
    $('#loadModal').modal('hide');
    updateToggleButton('relay1', data.relay1.state);
    updateToggleButton('relay2', data.relay2.state);
    updateToggleButton('relay3', data.relay3.state);
    updateToggleButton('relay4', data.relay4.state);
    updateToggleButton('relay5', data.relay5.state);
    updateToggleButton('relay6', data.relay6.state);
    updateToggleButton('relay7', data.relay7.state);
    updateToggleButton('relay8', data.relay8.state);
    $('#status').html('Ready');
  });
}
