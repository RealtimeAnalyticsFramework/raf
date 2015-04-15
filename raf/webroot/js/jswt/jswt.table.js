;
(function ($) {
    function refreshCheckbox($checkbox) {
        if ($checkbox.hasClass("fa-square-o")) {
            $checkbox.removeClass("fa-square-o").addClass("fa-check-square-o");
        } else {
            $checkbox.removeClass("fa-check-square-o").addClass("fa-square-o");
        }
    }

    $.fn.jswt_table_selected = function () {
        var selected = [];
        $.each($(this).find("i.fa-check-square-o"), function () {
            selected.push($(this).data("context"));
        });
        return selected;
    };

    $.fn.jswt_table = function (options) {
        options = $.extend(true, {}, $.fn.jswt_table.defaults, options);

        return this.each(function () {
            var elem = this, $elem = $(elem);
            var $table = $($(options.template).render(options));
            // empty container first, add by LT
            $elem.empty().append($table);
            var bodyEle = $table.find("tbody");
            var renderer = function (data) {
                // add by LT
                data = options.formator(data);
                $.each(data[options.root], function (i, obj) {
                    var trEle = $("<tr>");
                    $.each(options.headers, function (i, head) {
                        if (head.checkbox) {
                            var $checkbox = $('<i class="fa fa-lg fa-square-o"></i>');
                            $checkbox.data("context", obj);
                            trEle.append($("<td class='text-center'>").append($checkbox));
                        } else {
                            trEle.append($("<td>").append(obj[head.attr]));
                        }
                    });
                    bodyEle.append(trEle);
                    trEle.click(function (event) {
                        $(this).addClass('highlight').siblings().removeClass('highlight');
                        refreshCheckbox($(this).find("i.fa"));
                        options.onclickrow(i, obj);
                    });
                });
            };
            if (options.data) {
                renderer(options.data);
            }
            else if (options.url != "#") {
                $.post(options.url, JSON.stringify(options.post_data), renderer, "json")
                .success(function() { })
                .error(function() { })
                .complete(function() { });
            }
        });
    };

    $.fn.jswt_table.defaults = {
        url: "#",
        post_data: {},
        root: 'black',
        data: null,
        headers: [],
        template: "#tableTemplate",
        formator : function (d) {
            return d;
        },
        onclickrow: function (index, context) {
        }
    };
})(jQuery);