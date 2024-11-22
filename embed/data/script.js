// Scanning simulation
scanButton.addEventListener("click", () => {
    // Disable button and change text to "Scanning..."
    scanButton.textContent = "Scanning...";
    // scanButton.style.backgroundColor = "#d3d3d3";
    scanButton.disabled = true;  // Disable the button during scan
    
    // Change all sensor cards, result section to gray
    document.querySelectorAll('.card').forEach(card => card.style.backgroundColor = "#d3d3d3");
    resultSection.style.backgroundColor = "#d3d3d3";
    
    // Show "scanning..." in result section
    resultContent.innerHTML = `
      <h2>RESULT</h2>
      <div id="resultStatus">
        <p class="percentage">Scanning...</p>
        <p class="status">Please wait</p>
      </div>
    `;
    fetch("/scan")
        .then(response => {
            if (!response.ok) {
            console.log(2);

                throw new Error(`HTTP error! Status: ${response.status}`);
            }
            console.log(1);
            return response.json(); // Parse JSON response
        })
        .then(data => {
            // After scan, re-enable the button and reset text
            scanButton.textContent = "Start Scan";
            scanButton.disabled = false;  // Re-enable the button

            // Update result and sensor cards with new data and colors
            displayResults(data);
        })
        .catch(error => {
            console.error("Error:", error);

            // Show error message in result section
            resultContent.innerHTML = `
              <h2>RESULT</h2>
              <div id="resultStatus">
                <p class="percentage">Error</p>
                <p class="status">Failed to scan</p>
              </div>
              <p id="resultMessage" class="message">Could not retrieve data from the server.</p>
            `;

            // Reset button text
            scanButton.textContent = "Start Scan";
            scanButton.disabled = false;  // Re-enable the button
        });
  });
const talkingButton = document.getElementById('talkingbutton');
let recognition;

if ('webkitSpeechRecognition' in window) {
  recognition = new webkitSpeechRecognition(); // Use webkitSpeechRecognition for Chrome and other modern browsers
  recognition.continuous = true;  // Keep listening until manually stopped
  recognition.interimResults = false; // We only want final results

  recognition.onstart = () => {
    talkingButton.innerHTML = "Listening..."; // Change button text when listening
    talkingButton.disabled = true;
  };

  recognition.onerror = (event) => {
    talkingButton.innerHTML = "Error occurred, click to retry"; // Error handling
    talkingButton.disabled = false;
    console.error("Speech recognition error:", event.error);
  };

  recognition.onend = () => {
    talkingButton.innerHTML = "Click here<br>and Start Talking"; // Reset the button when recognition ends
    talkingButton.disabled = false;
  };

  //-----------------------------

  recognition.onresult = (event) => {
    // Get the transcript of what was said
    const spokenWord = event.results[event.resultIndex][0].transcript.toLowerCase();

    // Check if the spoken word matches any of the sensor names
    if (spokenWord.includes("humidity")) {
      const humidity = document.getElementById("humidityValue").textContent;
      displayDetectedWord("Humidity", humidity);
      recognition.stop(); // Stop recognition after detecting a word
    } else if (spokenWord.includes("dust")) {
      const dust = document.getElementById("dustValue").textContent;
      displayDetectedWord("Dust", dust);
      recognition.stop();
    } else if (spokenWord.includes("temperature")) {
      const temperature = document.getElementById("temperatureValue").textContent;
      displayDetectedWord("Temperature", temperature);
      recognition.stop();
    } else if (spokenWord.includes("brightness")) {
      const brightness = document.getElementById("brightnessValue").textContent;
      displayDetectedWord("Brightness", brightness);
      recognition.stop();
    }
  };

  // Function to display the detected word and its corresponding value
  function displayDetectedWord(word, value) {
    alert(`Detected word: ${word}\nValue: ${value}`);
  }

  // Start listening when the button is clicked
  talkingButton.addEventListener('click', () => {
    recognition.start();
  });
} else {
  alert("Speech recognition is not supported in this browser."); 
}

  
  // Function to generate random sensor data
  function generateRandomData() {
    return {
      humidity: Math.floor(Math.random() * (100 - 30) + 30), // Random humidity between 30% and 100%
      temperature: Math.floor(Math.random() * (45 - 15) + 15), // Random temperature between 15°C and 45°C
      dust: (Math.random() * (20 - 0)).toFixed(2), // Random dust value between 0.1 mg/m³ and 10 mg/m³
      brightness: Math.floor(Math.random() * (50 - 0)), // Random brightness between 0 and 400 nm
    };
  }
  
//   // Function to update results and sensor cards
//   function displayResults(data) {
//     const { humidity, temperature, dust, brightness } = data;
  
//     // Calculate sleep score based on sensor values
//     const sleepScore = calculateSleepScore(humidity, temperature, dust, brightness);
    
//     // Update result section
//     resultContent.innerHTML = `
//       <h2>RESULT</h2>
//       <div id="resultStatus">
//         <p class="percentage">${sleepScore}%</p>
//         <p class="status">${getStatus(sleepScore)}</p>
//       </div>
//       <p id="resultMessage" class="message">${getMessage(sleepScore)}</p>
//     `;
  
//     // Update sensor cards with random values
//     document.getElementById("humidityValue").textContent = humidity;
//     document.getElementById("temperatureValue").textContent = temperature;
//     document.getElementById("dustValue").textContent = dust;
//     document.getElementById("brightnessValue").textContent = brightness;
  
//     // Update colors of the sensor cards based on value
//     updateCardColor("humidity", humidity);
//     updateCardColor("temperature", temperature);
//     updateCardColor("dust", dust);
//     updateCardColor("brightness", brightness);
//   }

  // Function to update results and sensor cards, including sensor card color changes
function displayResults(data) {
    const { humidity, temperature, dust, brightness } = data;
  
    // Calculate sleep score based on sensor values
    const sleepScore = calculateSleepScore(humidity, temperature, dust, brightness);
    
    // Update result section content
    // resultContent.innerHTML = `
    //   <h2>RESULT</h2>
    //   <div id="resultStatus">
    //     <p class="percentage">${sleepScore}%</p>
    //     <p class="status">${getStatus(sleepScore)}</p>
    //   </div>
    //   <p id="resultMessage" class="message">${getMessage(sleepScore)}</p>
    // `;
    resultContent.innerHTML = `
    <h2>RESULT</h2>
    <div id="resultStatus">
        <p class="percentage">${sleepScore}%</p>
        <p class="status">${getStatus(sleepScore)}</p>
    </div>
    <p id="resultMessage" class="message">${getMessage(sleepScore, humidity, temperature, dust, brightness)}</p>
    `;
  
    // Update sensor cards with random values
    document.getElementById("humidityValue").textContent = humidity;
    document.getElementById("temperatureValue").textContent = temperature;
    document.getElementById("dustValue").textContent = dust;
    document.getElementById("brightnessValue").textContent = brightness;
  
    // Update colors of the sensor cards based on value
    updateCardColor("humidity", humidity);
    updateCardColor("temperature", temperature);
    updateCardColor("dust", dust);
    updateCardColor("brightness", brightness);
    }

  
  // Function to calculate the sleep score (out of 100)
  function calculateSleepScore(humidity, temperature, dust, brightness) {
    let score = 0;
  
    // Humidity: Ideal range is 40-60%
    let hscore = Math.max(0, 100 - Math.abs(humidity - 50) * (100 / 50)) * 0.25;
  
    // Temperature: Ideal is 25°C
    // score += (100 - Math.abs(25 - temperature)) * 0.25;
    let tscore = Math.max(0, 100 - Math.abs(25 - temperature) * 6) * 0.25;
  
    // Dust: Ideal is below 1 mg/m³
    let dscore = (dust < 12 ? 100 : Math.max(0, 100 - dust * 5)) * 0.25;
  
    // Brightness: Ideal is under 150 lux
    // score += (brightness < 150 ? 100 : Math.max(0, 100 - (brightness - 150) / 10)) * 0.25;\
    // let bscore = (brightness < 150 ? 100 : Math.max(0, 100 - (brightness - 150) / (400 - 150) * 100)) * 0.25;
    let bscore = (brightness < 10 ? 100 : Math.max(0, 100 - (brightness - 10) / (50 - 10) * 100)) * 0.25;

    console.log(hscore, tscore, dscore, bscore)

    score += hscore + tscore + dscore + bscore
  
    return Math.round(score);
  }
  
  // Function to determine the status message based on sleep score
  function getStatus(sleepScore) {
    if (sleepScore >= 80) {
      return "Great for Sleep! 😴";
    } else if (sleepScore >= 60) {
      return "Decent, but could be better.";
    } else {
      return "Not ideal for sleep. 😟";
    }
  }
  
//   // Function to generate the message based on sleep score
//   function getMessage(sleepScore) {
//     if (sleepScore >= 80) {
//       return "Your environment is perfect for a good night’s sleep!";
//     } else if (sleepScore >= 50) {
//       return "Your environment is okay, but some improvements could help.";
//     } else {
//       return "Your environment isn’t great for sleep. Consider making adjustments!";
//     }
//   }

    function getMessage(sleepScore, humidity, temperature, dust, brightness) {
        let adjustments = [];

        // Check humidity: Ideal range is 40-60%
        if (humidity < 35 || humidity > 65) {
            adjustments.push("adjusting humidity (40-60%)");
        }

        // Check temperature: Ideal is 25°C
        if (temperature < 22 || temperature > 28) { // Wider acceptable range for temperature
            adjustments.push("maintaining temperature near 25°C");
        }

        // Check dust: Ideal is below 1 mg/m³
        if (dust >= 12) {
            adjustments.push("reducing the dust levels to less than 12 µg/m³");
        }

        // Check brightness: Ideal is under 150 lux
        if (brightness >= 10) {
            adjustments.push("lowering the brightness to less than 10 lux");
        }

        // Generate the message
        if (sleepScore >= 80) {
            return "Your environment is perfect for a good night’s sleep!";
        } else if (sleepScore >= 60) {
            return `Your environment is okay, but some improvements could help, like: <br>${adjustments.join("<br>")}.`;
        } else {
            return `Your environment isn’t great for sleep. Consider:  <br>${adjustments.join("<br>")}.`;
        }
    }

  
//   // Function to update the card color smoothly based on the value
//   function updateCardColor(sensor, value) {
//     const card = document.querySelector(`#${sensor}Value`).closest('.card');
//     let color = "#f0f0f0"; // Light grey by default (neutral)
  
//     // Humidity: Ideal range is 40-60%
//     if (sensor === "humidity") {
//       let greenValue = Math.min(255, Math.abs(50 - value) * 4); // Smooth gradient from red to green
//       let redValue = Math.min(255, Math.abs(50 - value) * 4);
//       color = `rgb(${greenValue}, ${255 - greenValue}, ${0})`;  // Green to red gradient
//     } 
//     // Temperature: Ideal is 25°C
//     else if (sensor === "temperature") {
//       let diff = Math.abs(25 - value);
//       color = diff < 3 ? "rgb(0,255,0)" : diff < 5 ? "rgb(255,255,0)" : "rgb(255,0,0)";
//     } 
//     // Dust: Ideal is below 1 mg/m³
//     else if (sensor === "dust") {
//       color = value < 1 ? "rgb(0,255,0)" : "#ffada0"; // Green for low dust, red for high dust
//     } 
//     // Brightness: Ideal is under 150 lux
//     else if (sensor === "brightness") {
//       color = value < 150 ? "rgb(0,255,0)" : "#ffada0"; // Green for low brightness, red for high brightness
//     }
  
//     // Apply background color change to the card
//     card.style.transition = "background-color 0.5s ease";  // Smooth transition
//     card.style.backgroundColor = color;
//   }

  // Function to update the card color smoothly based on the value
function updateCardColor(sensor, value) {
    const card = document.querySelector(`#${sensor}Value`).closest('.card');
    let color = "#f0f0f0"; // Light grey by default (neutral)

    // Humidity: Ideal range is 40-60%
    if (sensor === "humidity") {
        let greenValue = Math.min(255, Math.abs(50 - value) * 4); // Smooth gradient from red to green
        let redValue = Math.min(255, Math.abs(50 - value) * 4);
        color = `rgb(${greenValue}, ${255 - greenValue}, ${100})`;  // Green to red gradient
    } 
    // Temperature: Ideal is 25°C
    else if (sensor === "temperature") {
        let diff = Math.abs(25 - value);
        // Apply a gradient from blue (cool) to green (ideal) to red (hot)
        color = `rgb(${Math.min(255, diff * 17)}, ${255 - Math.min(255, diff * 17)}, 100)`; 
    } 
    // Dust: Ideal is below 1 mg/m³
    else if (sensor === "dust") {
        // // If the dust level is lower, it's green; if higher, it's red.
        // let greenValue = Math.max(0, 255 - Math.min(255, value * 25));  // Green for low dust
        // let redValue = Math.min(255, value * 25);  // Red for high dust
        // color = `rgb(${redValue}, ${greenValue}, 0)`;  // Green to red gradient
        if (value < 12) {
            color = "rgb(0,255,100)"; // Green for ideal brightness
        } else {
            // Transition from green to red as brightness increases
            let diff = Math.min(255, (value - 12) * 36); // Scale excess brightness to 0-255
            color = `rgb(${diff}, ${255 - diff}, 100)`; // Gradual transition from green to red
        }
    } 
    // Brightness: Ideal is under 150 lux
    else if (sensor === "brightness") {
        // Smooth gradient for brightness: ideal is under 150
        if (value < 10) {
            color = "rgb(0,255,100)"; // Green for ideal brightness
        } else {
            // Transition from green to red as brightness increases
            let diff = Math.min(255, (value - 10) * 6.375); // Scale excess brightness to 0-255
            color = `rgb(${diff}, ${255 - diff}, 100)`; // Gradual transition from green to red
        }
    }

    // Apply background color change to the card
    card.style.transition = "background-color 0.5s ease";  // Smooth transition
    card.style.backgroundColor = color;
}

function startTalking() {
    const talkingButton = document.getElementById('talkingbutton');
    talkingButton.innerHTML = "Talk something...";
    talkingButton.disabled = true;

    setTimeout(() => {
      talkingButton.innerHTML = "Click here<br>and Start Talking";
      talkingButton.disabled = false;
    }, 3000); // Reset after 3 seconds
  }