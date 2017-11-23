
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
    updateToggleButton('relay1', data.relay1.state);
    updateToggleButton('relay2', data.relay2.state);
    updateToggleButton('relay3', data.relay3.state);
    updateToggleButton('relay4', data.relay4.state);
    updateToggleButton('relay5', data.relay5.state);
    $('#servo1Name').text(data.servo1.name);
    $('#servo2Name').text(data.servo2.name);
    $('#servo3Name').text(data.servo3.name);
    updateToggleButton('servo1', data.servo1.state);
    updateToggleButton('servo2', data.servo2.state);
    updateToggleButton('servo3', data.servo3.state);
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
    updateToggleButton('servo1', data.servo1.state);
    updateToggleButton('servo2', data.servo2.state);
    updateToggleButton('servo3', data.servo3.state);
    $('#status').html('Ready');
  });
}
