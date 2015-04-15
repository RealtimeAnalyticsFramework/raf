;
(function ($) {
    $.fn.jswt_panel = function (options) {
        options = $.extend(true, {}, $.fn.jswt_panel.defaults, options);

        var elem = this, $elem = $(elem);

        var $body = $($(options.template).render(options));
        $elem.append($body);
        return $body.find(".panel-body");
    };

    $.fn.jswt_panel.defaults = {
        icon_class: "",
        title: "",
        style: "",
        template: "#panelTemplate"
    };
})(jQuery);