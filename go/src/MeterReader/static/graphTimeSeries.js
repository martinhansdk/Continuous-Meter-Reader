// Code for plotting graphs

/*
  Some explanation:
  Here the canvasjs HTML5 charting plugin is adapted to React.
  We use these together with React's componentDidMount, componentDidUpdate and componentWillUnmount.

  The React component does not save any state. This is all passed in through properties.
*/

/* Functions for canvasjs */
canvasjs = {};
canvasjs.chart = null;
canvasjs.create = function(node, props, chartdata) {
    this.chart = new CanvasJS.Chart(node, {
	zoomEnabled: true,

	title: {
	    text: props.title
	},
	data: [ // array of data series
	    { // dataSeries object
		type: "area",
		xValueType: "dateTime",
		dataPoints: chartdata
	    }
	]
    });
    this.chart.render()
};

canvasjs.update = function(props, data) {
    //console.log("graphTimeSeries-canvasjs.update-data: ", data);
    this.chart.options.title.text = props.title;
    this.chart.options.data[0].dataPoints = data;
    this.chart.render();
};

canvasjs.destroy = function(node) {
  // Any clean-up would go here
  // in this example there is nothing to do
};

/* The actual React component */
var GraphTimeSeries = React.createClass({
    propTypes: {
	data: React.PropTypes.array,
	title: React.PropTypes.string
    },

    componentDidMount: function() {
	var node = ReactDOM.findDOMNode(this);
	canvasjs.create(node, {
	    title: this.props.title
	}, this.getChartState());
    },

    componentDidUpdate: function() {
	canvasjs.update({
	    title: this.props.title
	}, this.getChartState());
    },

    componentWillUnmount: function() {
	var node = ReactDOM.findDOMNode(this);
	canvasjs.destroy(node);
    },

    getChartState: function() {
	return this.props.data;
    },

    render: function() {
	return (
	    <div style={{height: this.props.height}}></div>
	);
    }
});
