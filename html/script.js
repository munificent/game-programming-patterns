$(document).ready(function() {
  $(window).resize(refreshAsides);

  // Since we may not have the height correct for the images, adjust the asides
  // too when an image is loaded.
  $('img').on('load', function() {
    refreshAsides();
  });

  // On the off chance the browser supports the new font loader API, use it.
  if (document.fontloader) {
    document.fontloader.notifyWhenFontsReady(function() {
      refreshAsides();
    });
  }

  // Lame. Just do another refresh after a second when the font is *probably*
  // loaded to hack around the fact that the metrics changed a bit.
  window.setTimeout(refreshAsides, 200);

  refreshAsides();
  loadInitialTheme();
});

function refreshAsides() {
  // Don't position them if they're inline.
  if ($(document).width() < 800) return;

  // Vertically position the asides next to the span they annotate.
  $("aside").each(function() {
  var aside = $(this);

  // Find the span the aside should be anchored next to.
  var name = aside.attr("name");
  var span = $("span[name='" + name.replace("'", "\\'") + "']");
  if (span == null) {
    window.console.log("Could not find span for '" + name + "'");
    return;
  }

  if (span.position()) {
    aside.offset({top: span.position().top - 3});
  }
  });
}

function loadInitialTheme() {
  const theme = localStorage.getItem('theme');
  if(theme === 'dark') {
    toggleTheme();
  }
}

function toggleTheme() {
  document.body.classList.toggle('dark');

  if(document.body.classList.contains('dark')) {
    document.querySelector('.theme-toggler').setAttribute('title', 'light theme');
    localStorage.setItem('theme', 'dark')
  } else {
    document.querySelector('.theme-toggler').setAttribute('title', 'dark theme');
    localStorage.removeItem('theme')
    }
}
