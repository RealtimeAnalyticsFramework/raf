;
(function ($) {
    $.jswt_dashboard = {
        init: function (options) {
            this.init_header(options);
            this.init_toggler(options);
            this.init_menu(options);
        },
        init_header: function (options) {
            $("#header").html($("#headerTemplate").render(options));
        },
        init_toggler: function (options) {
            $('.navigation-toggler').bind('click', function () {
                var $body = $('body');
                if (!$body.hasClass('navigation-small')) {
                    $body.addClass('navigation-small');
                } else {
                    $body.removeClass('navigation-small');
                }
            });
        },
        init_menu: function (options) {
            var $menu = $("#menu");
            $menu.html($("#menuTemplate").render(options));

            var nav = $("#dashboard-nav");
            var iEle = nav.find("i");
            var aEle = nav.find("a");
            var liEle = nav.find("li.active");

            function setNav(val) {
                if (val.icon_class) {
                    iEle.attr("class", val.icon_class);
                }
                aEle.html("&nbsp;" + val.label);
                liEle.html(val.desc);
                if (val.onclick) {
                    val.onclick();
                }
            }

            $.each($menu.find("li"), function (i, ele) {
                var value = options.menus[i];
                if (i == 0) {
                    $(this).addClass("active open");
                    setNav(value);
                }
                $(this).click(function (event) {
                    $(this).addClass('active open').siblings().removeClass('active open');
                    setNav(value);
                });
            });
        }
    };
})(jQuery);