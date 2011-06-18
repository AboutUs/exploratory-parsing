var samples = function(offsets) {
    location.href = location.href + '/offsets?offsets=' + offsets.join(',');
};

$(document).ready(function() {
  resizeSvgObject();

  $(".run").hover(
    function() {
      if( $( this ).hasClass( 'deleted' )) return false;
      $( this ).addClass( 'hover' );
      $.ajax({
        url: "/runs/" + $(this).attr('id') + "/diff",
        success: function(data) {
          showPreview( data );
        }
      });
    },
    function() {
      $( this ).removeClass( 'hover' );
    }
  );

  $(".run").click(function() {
    $('.run').removeClass('current');
    $(this).addClass('current');
    $.ajax({
      url: "/runs/" + $(this).attr('id') + "/leg",
      success: function(data) {
        $("#grammar").text( data );
        $( "#preview" ).hide( 0 );
        $( this ).removeClass( 'hover' );
      }
    });
  });
  $(".runs").hover(
    function(){},
    function() {
      $( "#preview" ).hide( 0 );
    }
  );

  $( '#preview' ).mouseover( function() {
    $( '#preview' ).hide();
  })

  $( '.delete' ).click( function() {
    var row = $( this ).closest( '.run' );
    row.addClass( 'deleted' );
    $.ajax({
      type: 'POST',
      url: "/runs/" + row.attr('id') + "/delete",
      dataType: "text",
      success: function(data) {
        $( "#preview" ).hide( 0 );
      }
    });
  });
});

function resizeSvgObject() {
  if( $.browser.webkit ) $( '#dot' )
    .css( 'width', '100%' )
    .css( 'height', '90%' );
}

function showPreview( data ) {
  $( "#preview div" ).html( data );
  var pos = $( "#grammar" ).position();
  $( "#preview" )
    .css( 'top', pos.top + 6 )
    .css( 'left', pos.left - 6 )
    .css( 'width', $( "#grammar" ).css( 'width' ))
    .css( 'height', $( "#grammar" ).css( 'height' ))
    .show();
}


/*
 * Scale all textareas dynamically on the page
 * Requires JQuery as per https://gist.github.com/117849
 */
function scaleTextareas() {
  $('textarea.scaled').each(function(i, t){
    var m = 0;
    $($(t).val().split("\n")).each(function(i, s){
      m += (s.length/(t.offsetWidth/10)) + 1.5;
    });
    t.style.height = Math.floor(m + 6) + 'em';
  });
  setTimeout(scaleTextareas, 1000);
};
$(document).ready(function(){
  scaleTextareas();
});
