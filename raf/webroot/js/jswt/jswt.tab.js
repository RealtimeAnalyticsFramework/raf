;
(function ($) {
    $.fn.jswt_tab_refresh = function () {
        return this.each(function () {
            $(this).find("li.active").click();
        });
    };

    $.fn.jswt_tab_context = function (context) {
        return this.each(function () {
            $(this).data("context", context);
        });
    };

    $.fn.jswt_tab = function (options) {
        options = $.extend(true, {}, $.fn.jswt_tab.defaults, options);

        return this.each(function () {
            var elem = this, $elem = $(elem);

            var $tabs = $($(options.template).render(options));
            $elem.append($tabs);

            function clickTab(config, context) {
                if (config.onclick) {
                    config.onclick(config, context);
                }
            }

            $.each($tabs.find("li"), function (i, ele) {
                var tabConfig = options.tabs[i];
                if (i == 0) {
                    clickTab(tabConfig, $tabs.data("context"));
                }
                $(this).click(function (event) {
                    clickTab(tabConfig, $tabs.data("context"));
                });
            });
        });
    };

    $.fn.jswt_tab.defaults = {
        id: undefined,
        tabs: [],
        template: "#tabTemplate"
    };
})(jQuery);