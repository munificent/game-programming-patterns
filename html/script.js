// From: http://www.quirksmode.org/js/cookies.html.
function createCookie(days) {
  var date = new Date();
  date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
  document.cookie =
      "hideinprogress=true; expires=" + date.toGMTString() + "; path=/";
}

$(document).ready(function() {
  $(window).resize(refreshAsides);

  $(".dismiss").show().click(function() {
    createCookie(10);

    $(".in-progress").hide();
    refreshAsides();
  });

  $(".nav").click(function() {
    $(".nav").toggleClass("expanded");
  });

  if (document.cookie.indexOf("hideinprogress") == -1) {
    $(".in-progress").show();
  }

  // Since we may not have the height correct for the images, adjust the asides
  // too when an image is loaded.
  $('img').load(function() {
    refreshAsides();
  });

  // On the off chance the browser supports the new font loader API, use it.
  if (document.fontloader) {
    document.fontloader.notifyWhenFontsReady(function() {
      refreshAsides();
    });
  }

  // Lame. Just do another refresh after a second when the font is *probably*
  // loaded to hack around the fact that the metrics changed a bit and the
  // help box has gotten a line taller.
  window.setTimeout(refreshAsides, 200);

  refreshAsides();
});

function refreshAsides() {
  if ($(document).width() > 700) {
    sidebarAsides();
  } else {
    inlineAsides();
  }
}

// Moves the asides into the main content column.
function inlineAsides() {
  $(".page").removeClass("sidebar");
  $(".nav").removeClass("sidebar");
  $("aside").removeClass("sidebar");
}

// Moves the asides to a second column on the right.
function sidebarAsides() {
  $(".page").addClass("sidebar");
  $(".nav").addClass("sidebar");
  $("aside").each(function() {
    var aside = $(this);

    // Find the span the aside should be anchored next to.
    var name = aside.attr("name");
    var span = $("span[name='" + name + "']");
    if (span == null) {
      window.console.log("Could not find span for '" + name + "'");
      return;
    }

    aside.addClass("sidebar");
    aside.offset({top: span.position().top - 3});
  });
}