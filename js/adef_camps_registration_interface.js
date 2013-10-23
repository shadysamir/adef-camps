(function($) {
  var timer;
  timer = setInterval(getSlots, 10000);
  getSlots();
  $().ready(function() {
    //prepare sessions interaction
    $('.tooltip').hide();
    $('.session').mouseover(function() {
      if (getClassByPrefix($(this), 'id-') == 'id-') {
        return;
      }
      $('.' + getClassByPrefix($(this), 'id-')).addClass('repeated');
      if ($('.' + getClassByPrefix($(this), 'workshop-') + '.selected').length == 0) {
        $('.' + getClassByPrefix($(this), 'workshop-')).not('.' + getClassByPrefix($(this), 'id-')).addClass('required');
      }
      $(this).removeClass('repeated');
      if (!$(this).hasClass('disabled')) {
        $(this).children('.tooltip').show().children().show();
      }
    }).mouseleave(function() {
      $('.' + getClassByPrefix($(this), 'id-')).removeClass('repeated');
      if ((!$(this).hasClass('selected')) && $('.' + getClassByPrefix($(this), 'workshop-') + '.selected').length == 0) {
        $('.' + getClassByPrefix($(this), 'workshop-')).removeClass('required');
      }
      $(this).children('.tooltip').hide();
    }).click(function() {
      if (getClassByPrefix($(this), 'id-') == 'id-') {
        return;
      }
      if ($(this).hasClass('disabled') && !($(this).hasClass('selected'))) {
        return;
      }
      if ($(this).hasClass('selected')) {
        $(this).removeClass('selected');
        $('.' + getClassByPrefix($(this), 'timecode-')).removeClass('disabled');
        if ($('.' + getClassByPrefix($(this), 'workshop-') + '.selected').not('.' + getClassByPrefix($(this), 'id-')).length > 0
                && $('.' + getClassByPrefix($(this), 'id-') + '.selected').length == 0) {
          $('.' + getClassByPrefix($(this), 'id-')).addClass('required');
        }
      } else {
        $('.' + getClassByPrefix($(this), 'timecode-')).addClass('disabled');
        $(this).addClass('selected');
        $('.' + getClassByPrefix($(this), 'id-')).removeClass('required');
      }
      updateSelected();
    }); //finished prepare sessions interaction

    //deal with selected sessions
    selected = $('input[name=timespace]').val();
    selected_array = selected.split(',');
    for (var i = 0; i < selected_array.length; i++) {
      $('.session.timespaceid-' + selected_array[i]).click();
    }
  });

  function getClassByPrefix(item, prefix) {
    classes = $(item).attr('class').split(" ");
    for (var i = 0; i < classes.length; i++) {
      if (classes[i].indexOf(prefix) == 0) {
        return classes[i];
      }
    }
    return '';
  }

  function updateSelected() {
    var selected = new Array();
    $('.session.selected').each(function(pos, session) {
      cssclasses = $(session).attr('class').split(' ');
      for (i = 0; i < cssclasses.length; i++) {
        if (cssclasses[i].substr(0, 11) == 'timespaceid') {
          timespaceid = cssclasses[i].replace('timespaceid-', '');
        }
      }
      selected[pos] = timespaceid;
    })
    $('input[name=timespace]').val(selected.toString());
  }

  function getSlots() {
    var url = '/?q=adefcamps/registration/slots';
    $.getJSON(url, function(json) {
      $.each(json, function(key, val) {
        if (val > 0) {
          message = Drupal.t("Remaining slots: @slots", {"@slots": val});
        } else {
          message = Drupal.t("No slots");
        }
        $('.timespaceid-' + key + ' .slots').html(message);
      });
    });
  }
}(jQuery));