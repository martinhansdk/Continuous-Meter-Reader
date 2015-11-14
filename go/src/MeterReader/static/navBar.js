/**
 * @jsx React.DOM
 */
var NavBar = React.createClass({
   render: function() {
      var navElements = this.props.navItems.map( function(title) {
        var navItem = <li><a href="#">{title}</a></li>
        return navItem;
      });

      return (
           <nav role="navigation">
             <ul>
               {navElements}
             </ul>
           </nav>
      );
    }
   });
