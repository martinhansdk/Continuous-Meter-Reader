// Code for the left hand side panel

var LeftPanel = React.createClass({
    render: function() {
	console.log("LeftPanel :", this.props.meterList);
	return (
  <div id="leftside" className="col-md-2">
    <div id="leftpane">
      <div className="panel panel-primary">
        <div className="panel-heading">
          <h3 className="panel-title">Panel title</h3>
        </div>
        <MeterList meterList={this.props.meterList} onUpdate={this.props.onUpdate}/>
      </div>
    </div>
  </div>
	);
    }
});

