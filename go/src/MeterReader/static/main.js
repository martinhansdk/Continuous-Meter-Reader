/*
UI build-up:

main
  leftpanel
    leftpane
      panel
        panel-heading
        meterlist
  rightpanel
    panel
      panel-heading
      graphbody

Data moves from 'main' into both panels and the state is maintained on 'main'
Clicking a meter in the left panel will affect the data shown in the right panel.

The two panels are stateless.
*/

//ReactDOM.render(<NavBar navItems={['Home', 'About', 'My Account', 'Contact']}/>, document.getElementById("navbar"));

var meters = [{meter:'1', name:'Vand', value:'2344' , unit:'m3'},
              {meter:'2', name:'Gas', value:'12344' , unit:'m3'},
              {meter:'3', name:'El', value:'232421' , unit:'kWh'} ];

var timedata = [
               ];

// Maintain state here.
var Main = React.createClass({
   // Set initial state
   getInitialState: function() {
       return {
	   meterSelection: 1
       };
   },

   // Mount socket and wait for "meter update" events.
   componentDidMount: function() {
       var that = this;
       this.socket = io();
       this.socket.on('meter update', function (data) {
           console.log("JSON recv:", data);
           that.addMeterUpdate(data);
       });
   },

   addMeterUpdate: function(update) {
       var data = this.props.meterList;
       var len  = data.length;
   
       console.log("props.data:", data);
   
       // Find the right meter and overwrite with the update.
       for (var i = 0; i < len; i++) {
           if(data[i].meter == update.meter) {
               data[i] = update;
           }
       }
       this.setState(data);
   },

    render: function() {
        console.log("Main");
	return (
	    <div>
		<LeftPanel meterList={this.props.meterList}/>
		<RightPanel />
	    </div>
	);
    }
});

/*
// create HiThere component
var HiThere = React.createClass({
    render: function () {
        return (
            <h1>Graphs go here</h1>
        );
    }
});
ReactDOM.render(<HiThere />, document.getElementById('main'));
*/

// Seed the initial state of the webpage. The remaining of the time socket.io does the work.
$.getJSON( "/api/currentabsolutevalues", function( meters ) {
//   ReactDOM.render(<MeterList meterList={meters}/>, document.getElementById("meterlist"));
   ReactDOM.render(<Main meterList={meters}/>, document.getElementById("main"));
});
