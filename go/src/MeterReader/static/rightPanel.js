// Code for the right hand side panel
var Panel = ReactBootstrap.Panel;

/*
var RightPanelBody = React.createClass({
    render: function() {
	// Style the div to 500px height because the canvas otherwise won't
	// look like it is inside the panel
	return (
        <div className="panel-body" style={{height:'500px'}}>
		<GraphTimeSeries data={this.props.data} title={this.props.title} />
        </div>
	);
    }
});
*/

var RightPanel = React.createClass({
    // Set initial state
    getInitialState: function() {
	//console.log("rightPanel-getInitialState ", this.props.meterID);
	return {
	    data: this.getData(this.props.meterID),
	};
    },

    /* Not too nice this one. Making GET operation synchronous, but I am not sure how to handle it AJAX style?? */
    getData: function(meterID) {
	//var that = this;
	var mydata = [];
	// http://stackoverflow.com/questions/13009755/getjson-synchronous
	$.ajaxSetup({
	    async: false
	});
	var jqxhr = $.getJSON( "/api/values/"+meterID )
	.done(function(data) {
            //console.log("rightPanel-getData-JSON recv: ",data);
	    //that.setState({ data : data });
	    mydata = data;
        })
	.fail(function() {
	    alert( "error: Failed to get graph data from server");
	    //that.setState({ data : [] });
	});
	$.ajaxSetup({
	    async: true
	});
	//console.log("returning data");
	return mydata;
    },
    componentDidMount: function() {
	//console.log("rightPanel-componentDidMount ", this.props.meterID);
	this.setState({data: this.getData(this.props.meterID)});
    },

    /* TODO: Move state to main?? */
    componentWillReceiveProps: function(nextProps) {
	//console.log("rightPanel-componentWillReceiveProps ", nextProps.meterID);
	this.setState({data: this.getData(nextProps.meterID)});
    },

    render: function() {
        const title = (
	    <h3>Graphs</h3>
	);
	//console.log("rightPanel-render", this.state);

	return (
          <div id="rightpane" className="col-md-8">
	    <Panel bsStyle="primary" header={title}>
		<GraphTimeSeries data={this.state.data} title={"Data for meter "+this.props.meterID} height="500px" />
	    </Panel>
          </div>
	);

//		{JSON.stringify(this.state.data, null, 2) }

/*
	return (
		<div id="rightpane" className="col-md-8">
  		  <div className="panel panel-primary">
 		    <div className="panel-heading">
		      <h3 className="panel-title">Panel title {this.props.meterID}</h3>
		    </div>
		    <RightPanelBody  data={this.state.data} title={"Data for meter "+this.props.meterID}/>
		  </div>
		</div>
	);
*/
    }
});
