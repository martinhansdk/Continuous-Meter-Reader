// Code for the right hand side panel
var Panel = ReactBootstrap.Panel;

var RightPanelBody = React.createClass({
    render: function() {
	return (
        <div id="graph_body" className="panel-body">
            <div styles="margin-bottom:10px; margin-left:20px" id="mylegend"></div>
            <div styles="display:block; float:left; width:20px; height:280px; padding-bottom:10px;" id="yaxis"></div>
            <div>
              <div styles="margin-left:20px;" id="graphbody"></div>
              <div styles="margin-left:20px" id="xaxis"></div>
              <div styles="margin-left:20px" id="previewSlider"></div>
            </div>
        </div>
	);
    }
});

var RightPanel = React.createClass({
    // Set initial state
    getInitialState: function() {
	return {
	    data: ''
	};
    },

    componentWillReceiveProps: function() {
	var data;
	var that=this;
	var jqxhr = $.getJSON( "/api/values/"+this.props.meterID )
	.done(function(data) {
            console.log(data);
	    that.setState({ data:data });
        })
	.fail(function() {
	    alert( "error: Failed to get graph data from server");
	    that.setState({ data:'' });
	});
/*
	.always(function() {
	    alert( "complete");
	});
*/
    },

    render: function() {
        const title = (
	    <h3>Graphs</h3>
	);

	return (
          <div id="rightpane" className="col-md-8">
	    <Panel bsStyle="primary" header={title}>
		{JSON.stringify(this.state.data, null, 2) }
	    </Panel>
          </div>
	);
/*
	return (
  <div id="rightpane" className="col-md-8">
      <div className="panel panel-primary">
        <div className="panel-heading">
          <h3 className="panel-title">Panel title {this.props.meterID}</h3>
        </div>
        <RightPanelBody />
      </div>
  </div>
	);
*/
    }
});
