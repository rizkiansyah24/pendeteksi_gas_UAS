/*************************************************
 * MQTT CONFIGURATION
 *************************************************/

const options = {

    username: "deteksi-gas",

    password: "doLZdHOo3Z1otXG3",

    clientId: "Dashboard_" + Math.random().toString(16).substring(2, 10),

    clean: true,

    reconnectPeriod: 3000,

    connectTimeout: 30000

};


/*************************************************
 * CONNECT MQTT
 *************************************************/

const client = mqtt.connect(
    "wss://deteksi-gas.cloud.shiftr.io:443",
    options
);


/*************************************************
 * MQTT CONNECTED
 *************************************************/

client.on("connect", () => {

    console.clear();

    console.log("====================================");
    console.log("MQTT CONNECTED");
    console.log("====================================");

    document.getElementById("mqttStatus").innerHTML = "Connected";
    document.getElementById("mqttStatus").className = "online";

    client.subscribe("gas/value");
    client.subscribe("gas/status");
    client.subscribe("system/status");

});


/*************************************************
 * MQTT RECONNECT
 *************************************************/

client.on("reconnect", () => {

    console.log("Reconnect MQTT...");

});


/*************************************************
 * MQTT OFFLINE
 *************************************************/

client.on("offline", () => {

    console.log("MQTT Offline");

    document.getElementById("mqttStatus").innerHTML = "Offline";
    document.getElementById("mqttStatus").className = "offline";

});


/*************************************************
 * MQTT ERROR
 *************************************************/

client.on("error", (error) => {

    console.error("MQTT Error");

    console.error(error);

});


/*************************************************
 * RECEIVE MESSAGE
 *************************************************/

client.on("message", (topic, message) => {

    const payload = message.toString();

    console.log(topic, payload);

    //----------------------------------------
    // Data Sensor
    //----------------------------------------

    if (topic === "gas/value") {

        try {

            const json = JSON.parse(payload);

            updateDashboard(
                json.gas,
                json.status
            );

        }
        catch (err) {

            console.error("JSON Error");

            console.error(err);

        }

    }

    //----------------------------------------
    // Status ESP32
    //----------------------------------------

    if (topic === "system/status") {

        document.getElementById("espStatus").innerHTML = "Online";

        document.getElementById("espStatus").className = "online";

    }

});


/*************************************************
 * BUTTON LED ON
 *************************************************/

document.getElementById("ledOn").addEventListener("click", () => {

    client.publish(
        "gas/control",
        "LED_ON"
    );

});


/*************************************************
 * BUTTON LED OFF
 *************************************************/

document.getElementById("ledOff").addEventListener("click", () => {

    client.publish(
        "gas/control",
        "LED_OFF"
    );

});