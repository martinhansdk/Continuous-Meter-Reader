  var myGraph = new Rickshaw.Graph({
    element: document.querySelector("#graphbody"),
    width: 500,
    height: 250,
    min: 0,
    max: 18,
    renderer: 'lineplot',
    series: [ 
    {
      name: "Vand",
      color: "steelblue",
      data: [
      {x: 0, y:10,},{x: 1, y:3,},{x: 2, y:8,},{x: 3, y:15,},{x: 4, y:12,},
      {x: 5, y:8,},{x: 6, y:3,},{x: 7, y:5,},{x: 8, y:2,},{x: 9, y:1,},{x: 10, y:4,},
      ]
    },
    {
      name: "El",
      color: "green",
      data: [
      {x: 0, y:5,},{x: 1, y:3,},{x: 2, y:8,},{x: 3, y:6,},{x: 4, y:3,},
      {x: 5, y:12,},{x: 6, y:13,},{x: 7, y:14,},{x: 8, y:12,},{x: 9, y:8,},{x: 10, y:9,},
      ]
    }
    ]
  });
  var xTicks = new Rickshaw.Graph.Axis.X({
    graph:myGraph,
    orientation: "bottom",
    element: document.querySelector("#xaxis")
  });
  var yTicks = new Rickshaw.Graph.Axis.Y({
    graph:myGraph,
    orientation: "left",
    element: document.querySelector("#yaxis")
  });
  var graphHover = new Rickshaw.Graph.HoverDetail({
    graph:myGraph
  });
  var myLegend = new Rickshaw.Graph.Legend({
    graph:myGraph,
    element: document.querySelector("#mylegend")
  });
  var previewSlider = new Rickshaw.Graph.RangeSlider.Preview({
    graph: myGraph,
    element: document.querySelector("#previewSlider")
  });
  myGraph.render();
