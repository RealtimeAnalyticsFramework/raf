;
(function ($) {
    $.fn.jswt_tree = function (options) {
        options = $.extend(true, {}, $.fn.jswt_tree.defaults, options);
        var elem = this, $elem = $(elem);
        var renderer = function(result) {
            result = options.formator(result);
            
            $elem.treeview({
                data: result.tree.nodes,
                onNodeSelected: options.onclick
            });
        };
        if (options.data) {
            renderer(options.data);
        }
        else if (options.url != "#") {
            $.post(options.url, JSON.stringify(options.post_data), renderer, "json");
        }
    };

    $.fn.jswt_tree.defaults = {
        url: "#",
        data: null,
        post_data: {},
        formator : function (d) {
            return d;
        },
        onclick: function (event, node) {
        }
    };
})(jQuery);