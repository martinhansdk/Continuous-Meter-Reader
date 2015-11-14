// Code for the right hand side panel
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
    render: function() {
	return (
  <div id="rightpane" className="col-md-8">
      <div className="panel panel-primary">
        <div className="panel-heading">
          <h3 className="panel-title">Panel title</h3>
        </div>
        <RightPanelBody />
      </div>
  </div>
	);
    }
});
