/**
 * @jsx React.DOM
 */
var MeterListEntry = React.createClass({
   handleClick: function(event) {
     console.log("clicked"+event);
   },

   render: function() {
        return <div onClick={this.handleClick()}>{this.props.data.name}: {this.props.data.value} {this.props.data.unit}</div>;
      }
  });

var MeterList = React.createClass({
   render: function() {
      console.log("render1:"+this.props.meterList);
      return (
         <div id="meterlist">
           {this.props.meterList.map( function(data) {
             return <MeterListEntry key={data.meter} data={data} />
            })}
         </div>
      );
    }
   });
