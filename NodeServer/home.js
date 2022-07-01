var weather = require('./weather.js')

document.addEventListener('alpine:init', () => {
  console.log("alpine:init")
  Alpine.store('logging', {    
    logs: ['a', 'b', 'c', 'd', 'e', 'f', 'g'],

    add(log_msg) {
      this.logs.push(log_msg);
    }
  }),
  Alpine.store('weather', {
    messages: [],
    
    init() {
      this.messages = new Array(0);

      let server_url = window.location.href + "api/data"
      fetch(server_url)
        .then(response => response.arrayBuffer())
        .then(function(dataArray) {
          const byteArray = new Uint8Array(dataArray);
          console.log(byteArray);          
          let msg = weather.weather.Data.decode(byteArray);
          console.log(msg);
          this.messages.push(msg);
        })

    },
  })
})

const labels = [
  'January',
  'February',
  'March',
  'April',
  'May',
  'June',
];

const data = {
  labels: labels,
  datasets: [{
    label: 'My First dataset',
    backgroundColor: 'rgb(255, 99, 132)',
    borderColor: 'rgb(255, 99, 132)',
    data: [0, 10, 5, 2, 20, 30, 45],
  }]
};

const config = {
  type: 'line',
  data: data,
  options: {}
};
