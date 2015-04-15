;
(function ($) {
    $.jswt = {
    };
    $.jswt.load_templates = function (callback) {
        var $link = $("link[rel='external']");
        var complete = 0;
        $link.each(function () {
            $(this).load($(this).attr("href"), function () {
                complete++;
                if (complete == $link.length) {
                    callback();
                }
            });
        });
    };
})(jQuery);