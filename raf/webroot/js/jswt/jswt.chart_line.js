;
(function ($) {
    function showTooltip(x, y, contents) {
        $('<div id="tooltip">' + contents + '</div>').css({
            position: 'absolute',
            display: 'none',
            top: y + 5,
            left: x + 15,
            border: '1px solid #333',
            padding: '4px',
            color: '#fff',
            'border-radius': '3px',
            'background-color': '#333',
            opacity: 0.80
        }).appendTo("body").fadeIn(200);
    }

    $.fn.jswt_chart_line = function (options) {
        options = $.extend(true, {}, $.fn.jswt_chart_line.defaults, options);
        var style_options = $.extend(true, {}, $.fn.jswt_chart_line.style_defaults, {xaxis: options.xaxis, yaxis: options.yaxis});

        return this.each(function () {
            var elem = this, $elem = $(elem);
            
            var renderer = function(data) {
                // format data, add by LT
                data = options.formator(data);
                // console.log(data);
                var charts = [];
                $.each(options.metrics, function (i, metric) {
                    var chart_data = [];
                    $.each(data[metric], function (i, obj) {
                        chart_data.push([obj[options.x], obj[options.y]]);
                    });
                    charts.push({
                        data: chart_data,
                        label: metric
                    });
                });

                var $chart = $("<div></div>").addClass("flot-placeholder");
                // empty the container first, added by LT
                $elem.empty().append($('<div class="flot-medium-container">').append($chart));
                $.plot($chart, charts, style_options);
                var previousPoint = null;
                $chart.bind("plothover", function (event, pos, item) {
                    $("#x").text(pos.x.toFixed(2));
                    $("#y").text(pos.y.toFixed(2));
                    if (item) {
                        if (previousPoint != item.dataIndex) {
                            previousPoint = item.dataIndex;
                            $("#tooltip").remove();
                            var x = item.datapoint[0].toFixed(2),
                                y = item.datapoint[1].toFixed(2);
                            showTooltip(item.pageX, item.pageY, options.tooltip(x, y));
                        }
                    } else {
                        $("#tooltip").remove();
                        previousPoint = null;
                    }
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
    $.fn.jswt_chart_line.defaults = {
        url: "#",
        post_data: {},
        data: null,
        x: "",
        xaxis: {},
        yaxis: {},
        y: "",
        metrics: [],
        formator: function (d) {
            return d;
        },
        tooltip: function (x, y) {
            return x + " = " + y
        }
    };

    $.fn.jswt_chart_line.style_defaults = {
        series: {
            lines: {
                show: true,
                lineWidth: 2,
                fill: true,
                fillColor: {
                    colors: [
                        {
                            opacity: 0.05
                        },
                        {
                            opacity: 0.01
                        }
                    ]
                }
            },
            points: {
                show: false
            },
            shadowSize: 2
        },
        grid: {
            hoverable: true,
            clickable: true,
            tickColor: "#eee",
            borderWidth: 0
        },
        colors: ["#d12610", "#37b7f3", "#52e136"]
    };
})(jQuery);