/**
 * @jsx React.DOM
 */
var ButtonGroup = ReactBootstrap.ButtonGroup;
var Button      = ReactBootstrap.Button;

var MeterListEntry = React.createClass({
    onClick: function(e) {
	console.log("clicked: ");
//	console.log(e) // SyntheticMouseEvent
//	console.log(ReactDOM.findDOMNode(this.refs.button)); // The DOM node
	console.log(this.props.data.meter); // What we actually need
	this.props.onUpdate(this.props.data.meter); // Update the event handler
   },

   render: function() {
       return (
          <Button bsStyle="info" block onClick={this.onClick} ref="button">{this.props.data.name}: {this.props.data.value} {this.props.data.unit}</Button>
       );
// A few experiments here with checkboxes. Doesn't work without warnings in the console.
// uses innerInput and the code marked "checkbox" below
/*
       return (
	       <label className="btn btn-info btn-block">
                 <input type="checkbox" autoComplete="off" dangerouslySetInnerHTML={this.innerInput()}></input>
	       </label>
       );
*/
//	         <input type="checkbox" autoComplete="off">{this.props.data.name}: {this.props.data.value} {this.props.data.unit}</input>
   }

//   innerInput: function () { return {__html: this.props.data.name+": "+this.props.data.value+" "+ this.props.data.unit } },
  });

var MeterList = React.createClass({
   render: function() {
      console.log("render1:"+this.props.meterList);
      return (
         <ButtonGroup vertical block>
           {this.props.meterList.map(
	       function(data) {
		   return <MeterListEntry key={data.meter} data={data} onUpdate={this.props.onUpdate}/>
               }, this) // Pass 'this' to access props in the map function
           }
         </ButtonGroup>
      )
/* Checkbox code:
      return (
	  <div className="btn-group" data-toggle="buttons">
              {this.props.meterList.map( function(data) {
		  return <MeterListEntry key={data.meter} data={data} />
              })}
	  </div>
      )
*/
    }
   });
