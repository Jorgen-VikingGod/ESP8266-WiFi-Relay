$('#btnRefStat').click(function() {
  // get current stats
  getCurrent();
  $('#btnRefStat').css('display', 'none');
  $('#btnRefStat2').css('display', 'block');
});

$('#btnRefStat2').click(function() {
  $('#btnRefStat2').css('display', 'none');
  $('#btnRefStat').css('display', 'block');
});

$('#btnSaveConf').click(function() {
  var adminpwd = $('#adminPwd').val();
  if (adminpwd === null || adminpwd === '') {
    alert('Administrator Password cannot be empty');
    return;
  }
  var datatosend = {
    command: 'configfile',
    hostname: $('#hostname').val(),
    adminpwd: adminpwd,
    ssid: $('#ssid').val(),
    wifipwd: $('#wifipwd').val(),
    relay1: { name: $('#relay1Name').val(),
              type: $('#relay1Type option:selected').val(),
              pin: $('#relay1Pin option:selected').val() },
    relay2: { name: $('#relay2Name').val(),
              type: $('#relay2Type option:selected').val(),
              pin: $('#relay2Pin option:selected').val() },
    relay3: { name: $('#relay3Name').val(),
              type: $('#relay3Type option:selected').val(),
              pin: $('#relay3Pin option:selected').val() },
    relay4: { name: $('#relay4Name').val(),
              type: $('#relay4Type option:selected').val(),
              pin: $('#relay4Pin option:selected').val() },
    relay5: { name: $('#relay5Name').val(),
              type: $('#relay5Type option:selected').val(),
              pin: $('#relay5Pin option:selected').val() },
    relay6: { name: $('#relay6Name').val(),
              type: $('#relay6Type option:selected').val(),
              pin: $('#relay6Pin option:selected').val() },
    relay7: { name: $('#relay7Name').val(),
              type: $('#relay7Type option:selected').val(),
              pin: $('#relay7Pin option:selected').val() },
    relay8: { name: $('#relay8Name').val(),
              type: $('#relay8Type option:selected').val(),
              pin: $('#relay8Pin option:selected').val() },
  };
  $.jpost(urlBase + 'settings/configfile', datatosend);
  updateSettingsDialog();
});

$('#btnBackupSet').click(function() {
  $('#downloadSet')[0].click();
});

$('#restoreSet').change(function() {
  //get file object
  var file = $('#restoreSet').prop('files')[0];
  if (file) {
    // create reader
    var reader = new FileReader();
    reader.readAsText(file);
    reader.onload = function(e) {
      var json;
      // browser completed reading file - display it
      try {
        json = JSON.parse(e.target.result);
        if (json.command === 'configfile') {
          var x = confirm('File seems to be valid, do you wish to continue?');
          if (x) {
            $.jpost(urlBase + 'settings/configfile', json);
            updateSettingsDialog();
          }
        }
      } catch (e) {
        alert('Not a valid backup file');
        return;
      }
    };
  }
});

function updateSettingsDialog() {
  $('#updateModal').modal('show');
  setTimeout(function(){
    location.reload();
    $('#updateModal').modal('hide');
  }, 10000);
}

function getCurrent() {
  $('#loadModal').modal('show');
  $.get(urlBase + 'settings/status', function(data) {
    $('#loadModal').modal('hide');
    $('#chip').text(data.chipid);
    $('#cpu').text(data.cpu + ' MHz');
    $('#heap').text(data.heap + ' Bytes');
    $('#heap').css('width', (data.heap*100)/81920 + '%');
    colorStatusbar($('#heap'));
    $('#flash').text(data.availsize + ' Bytes');
    $('#flash').css('width', (data.availsize*100)/1044464 + '%');
    colorStatusbar($('#flash'));
    $('#spiffs').text(data.availspiffs + ' Bytes');
    $('#spiffs').css('width', (data.availspiffs*100)/data.spiffssize + '%');
    colorStatusbar($('#spiffs'));
    $('#ssidstat').text(data.ssid);
    $('#ip').text(data.ip);
    $('#gate').text(data.gateway);
    $('#mask').text(data.netmask);
    $('#dns').text(data.dns);
    $('#mac').text(data.mac);
  });
}

function getAll() {
  $('#loadModal').modal('show');
  $.get(urlBase + 'settings/configfile', function(data) {
    $('#loadModal').modal('hide');
    $('#adminPwd').val(data.adminpwd);
    $('#hostname').val(data.hostname);
    $('#ssid').val(data.ssid);
    $('#wifipwd').val(data.wifipwd);
    var relay1Name = (data.relay1.name ? data.relay1.name : 'Relay 1');
    $('#relay1Name').val(relay1Name);
    $('#relay1Type').val(data.relay1.type).change();
    $('#relay1Pin').val(data.relay1.pin).change();
    var relay2Name = (data.relay2.name ? data.relay2.name : 'Relay 2');
    $('#relay2Name').val(relay2Name);
    $('#relay2Type').val(data.relay2.type).change();
    $('#relay2Pin').val(data.relay2.pin).change();
    var relay3Name = (data.relay3.name ? data.relay3.name : 'Relay 3');
    $('#relay3Name').val(relay3Name);
    $('#relay3Type').val(data.relay3.type).change();
    $('#relay3Pin').val(data.relay3.pin).change();
    var relay4Name = (data.relay4.name ? data.relay4.name : 'Relay 4');
    $('#relay4Name').val(relay4Name);
    $('#relay4Type').val(data.relay4.type).change();
    $('#relay4Pin').val(data.relay4.pin).change();
    var relay5Name = (data.relay5.name ? data.relay5.name : 'Relay 5');
    $('#relay5Name').val(relay5Name);
    $('#relay5Type').val(data.relay5.type).change();
    $('#relay5Pin').val(data.relay5.pin).change();
    var relay6Name = (data.relay6.name ? data.relay6.name : 'Relay 6');
    $('#relay6Name').val(relay6Name);
    $('#relay6Type').val(data.relay6.type).change();
    $('#relay6Pin').val(data.relay6.pin).change();
    var relay7Name = (data.relay7.name ? data.relay7.name : 'Relay 7');
    $('#relay7Name').val(relay7Name);
    $('#relay7Type').val(data.relay7.type).change();
    $('#relay7Pin').val(data.relay7.pin).change();
    var relay8Name = (data.relay8.name ? data.relay8.name : 'Relay 8');
    $('#relay8Name').val(relay8Name);
    $('#relay8Type').val(data.relay8.type).change();
    $('#relay8Pin').val(data.relay8.pin).change();
    var dataStr = 'data:text/json;charset=utf-8,' + encodeURIComponent(JSON.stringify(data, null, 2));
    $('#downloadSet').attr({download: data.hostname + '-settings.json',href: dataStr});
    $('#chip').text(data.chipid);
    $('#cpu').text(data.cpu + ' MHz');
    $('#heap').text(data.heap + ' Bytes');
    $('#heap').css('width', (data.heap*100)/81920 + '%');
    colorStatusbar($('#heap'));
    $('#flash').text(data.availsize + ' Bytes');
    $('#flash').css('width', (data.availsize*100)/1044464 + '%');
    colorStatusbar($('#flash'));
    $('#spiffs').text(data.availspiffs + ' Bytes');
    $('#spiffs').css('width', (data.availspiffs*100)/data.spiffssize + '%');
    colorStatusbar($('#spiffs'));
    $('#ssidstat').text(data.ssid);
    $('#ip').text(data.ip);
    $('#gate').text(data.gateway);
    $('#mask').text(data.netmask);
    $('#dns').text(data.dns);
    $('#mac').text(data.mac);
  });
}

function colorStatusbar(ref) {
  var width = ref.width();
  if (!ref.css('width').includes('%')) {
    var parentWidth = ref.offsetParent().width();
    var percent = 100*width/parentWidth;
  } else {
    var percent = width;
  }
  if (percent > 50) ref.attr('class','progress-bar progress-bar-success');
  else if (percent > 25) ref.attr('class','progress-bar progress-bar-warning');
  else ref.attr('class','progress-bar progress-bar-danger');
}
