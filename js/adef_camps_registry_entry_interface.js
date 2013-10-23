(function($) {
  $().ready(function() {
    $('#edit-track').change(function() {
      if($(this).val() != '0') {
        $('.form-item-goal').hide();
        $('#edit-goal').val('0');
      } else {
        $('.form-item-goal').show();
      }
    });
    $('#edit-goal').change(function() {
      if($(this).val() != '0') {
        $('.form-item-track').hide();
        $('#edit-track').val('0');
      } else {
        $('.form-item-track').show();
      }
    })
  });
}(jQuery));