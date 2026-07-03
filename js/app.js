/*************************************************
 * THEME
 *************************************************/

const themeButton = document.getElementById("themeButton");

themeButton.addEventListener("click", () => {

    document.body.classList.toggle("dark");

    if (document.body.classList.contains("dark")) {
        themeButton.innerHTML = "☀️";
    } else {
        themeButton.innerHTML = "🌙";
    }

});


/*************************************************
 * KOMPONEN DASHBOARD
 *************************************************/

const gasStatus = document.getElementById("gasStatus");
const gasValue = document.getElementById("gasValue");
const gasBar = document.getElementById("gasBar");
const historyTable = document.getElementById("history");
const alarmAudio = document.getElementById("alarmAudio");
const enableAudio = document.getElementById("enableAudio");


/*************************************************
 * VARIABEL GLOBAL
 *************************************************/
enableAudio.addEventListener("click", () => {

    alarmAudio.play()
    .then(() => {

        alarmAudio.pause();
        alarmAudio.currentTime = 0;

        audioEnabled = true;

        alert("Alarm berhasil diaktifkan.");

    })
    .catch(err => {

        console.log(err);

    });

});


let historyNumber = 1;

let lastStatus = "";

let lastValue = -1;

// Status Alarm
let alarmPlaying = false;

// Alarm dibisukan atau tidak
let alarmMuted = false;

let audioEnabled = false;
/*************************************************
 * UPDATE DASHBOARD
 *************************************************/

function updateDashboard(value, status)
{

    //---------------------------------------------
    // Nilai Sensor
    //---------------------------------------------
    gasValue.innerHTML = value + " ppm";

    //---------------------------------------------
    // Status Gas
    //---------------------------------------------
    if (status === "BOCOR")
    {
        gasStatus.innerHTML = "🔴 BOCOR";
        gasStatus.style.color = "#F44336";

        playAlarm();
    }
    else
    {
        gasStatus.innerHTML = "🟢 AMAN";
        gasStatus.style.color = "#00C853";

        stopAlarm();
    }

    //---------------------------------------------
    // Progress Bar
    //---------------------------------------------
    let percent = (value / 4095) * 100;

    if (percent > 100)
    {
        percent = 100;
    }

    gasBar.style.width = percent + "%";

    if (percent < 40)
    {
        gasBar.style.background = "#00C853";
    }
    else if (percent < 70)
    {
        gasBar.style.background = "#FFC107";
    }
    else
    {
        gasBar.style.background = "#F44336";
    }

    //---------------------------------------------
    // Tambahkan History jika ada perubahan
    //---------------------------------------------
    if (status !== lastStatus || Math.abs(value - lastValue) >= 100)
    {
        addHistory(status, value);

        lastStatus = status;

        lastValue = value;
    }

}


/*************************************************
 * TAMBAH HISTORY
 *************************************************/

function addHistory(status, value)
{

    const now = new Date();

    const waktu = now.toLocaleTimeString("id-ID");

    const row = document.createElement("tr");

    let badge = "";

    if (status === "BOCOR")
    {
        badge = "<span class='badge-danger'>🔴 BOCOR</span>";
    }
    else
    {
        badge = "<span class='badge-safe'>🟢 AMAN</span>";
    }

    row.innerHTML = `
        <td>${historyNumber}</td>
        <td>${waktu}</td>
        <td>${badge}</td>
        <td>${value} ppm</td>
    `;

    historyTable.prepend(row);

    historyNumber++;

    //---------------------------------------------
    // Maksimal 20 Riwayat
    //---------------------------------------------
    while (historyTable.rows.length > 20)
    {
        historyTable.deleteRow(20);
    }

}

/*************************************************
 * PLAY ALARM
 *************************************************/
function playAlarm()
{
    if (!audioEnabled)
    {
        return;
    }

    if (alarmMuted)
    {
        return;
    }

    if (!alarmPlaying)
    {
        alarmAudio.currentTime = 0;

        alarmAudio.play();

        alarmPlaying = true;
    }
}


/*************************************************
 * STOP ALARM
 *************************************************/
function stopAlarm()
{
    alarmAudio.pause();

    alarmAudio.currentTime = 0;

    alarmPlaying = false;
}


/*************************************************
 * TEST ALARM
 *************************************************/
function testAlarm()
{
    alarmAudio.currentTime = 0;

    alarmAudio.play();

    setTimeout(() => {

        alarmAudio.pause();

        alarmAudio.currentTime = 0;

    },3000);
}