;
(function ($) {
    $.fn.jswt_properties = function (options) {
        options = $.extend(true, {}, $.fn.jswt_properties.defaults, options);

        return this.each(function () {
            var elem = this, $elem = $(elem);
            var $table = $($(options.template).render(options));
            $elem.append($table);
            var bodyEle = $table.find("tbody");
            
            var renderer = function (data) {
                data = options.formator(data);
            
                $.each(options.properties, function (i, property) {
                    var trEle = $("<tr>");
                    trEle.append($('<td width="50%"></td>').append(property.label));
                    trEle.append($('<td width="50%"></td>').append(data[property.attr]));
                    bodyEle.append(trEle);
                });
            };
            
            if (options.data) {
                renderer(options.data);
            }            
            else if (options.url != "#") {
                $.post(options.url, JSON.stringify(options.post_data), renderer, "json");
            }
        });
    };
    $.fn.jswt_properties.defaults = {
        url: "#",
        data: null,
        post_data: {},
        formator : function (d) {
            return d;
        },
        properties: [],
        template: "#propertiesTemplate"
    };
})(jQuery);