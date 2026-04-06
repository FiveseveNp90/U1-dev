//


const Version = 0.9;

let opt = 0;
let hasStorage = false;
let hasInputs = false;
let hasOutputs = false;
let midiChannel = 16;
let statusTxt = document.getElementById("status");
let midiDdVal = "";
let outSel = document.getElementById("outList");
let inSel = document.getElementById("inList");
let dotCount = 0;
let inMatch = -1;
let outMatch = -1;

function errormsg(msg) {
    statusTxt.innerHTML = (msg);
    statusTxt.style.color = 'red';
}

WebMidi
    .enable()
    .then(onEnabled)
    .catch(err => errormsg(err));

function onEnabled() {
    outSel.replaceChildren();
    inSel.replaceChildren();
    if (WebMidi.outputs.length < 1) {
        hasOutputs = false;
        let el = document.createElement("option");
        el.textContent = "None";
        outSel.appendChild(el);
    } else {
        hasOutputs = true;
        for (let i = 0; i < WebMidi.outputs.length; i++) {
            let optn = WebMidi.outputs[i].name;
            let el = document.createElement("option");
            el.textContent = optn;
            el.value = i;
            outSel.appendChild(el);

            let optnLc = optn.toLowerCase();
            if (optnLc.includes("defect") || optnLc.includes("pico")) {
                outMatch = i;
                console.log("outMatch ", outMatch);
            }
        }
    }

    if (WebMidi.inputs.length < 1) {
        hasInputs = false;
        let el = document.createElement("option");
        el.textContent = "None";
        inSel.replaceChildren();
        inSel.appendChild(el);
    } else {
        hasInputs = true;
        for (let i = 0; i < WebMidi.inputs.length; i++) {
            let optn = WebMidi.inputs[i].name;
            let el = document.createElement("option");
            el.textContent = optn;
            el.value = i;
            inSel.appendChild(el);

            let optnLc = optn.toLowerCase();
            if (optnLc.includes("defect") || optnLc.includes("pico")) {
                inMatch = i;
                console.log("inMatch ", inMatch);
            }
        }
    }
    if (hasStorage) {
        if (localStorage.getItem("inputD") && localStorage.getItem("outputD")) {
            inSel.value = localStorage.getItem("inputD");
            outSel.value = localStorage.getItem("outputD");
        }
    }
    setInDevice();
    setOutDevice();
    statusTxt.innerHTML = "Ready";
    setTimeout(reqData, 500);
}

function reqData() {
    let outDevice = 0;
    if (opt == 0) {
        if (outMatch > -1) {
            outDevice = outMatch;
        }
    } else {
        outDevice = outSel.value;
    }
    if (WebMidi.outputs.length > 0) {
        if (midiDdVal == "") {
            for (let index = 1; index <= 16; index++) {
                WebMidi.outputs[outDevice].channels[index].sendControlChange(35, 123);
            }
            console.log("req init Data");
        } else {
            WebMidi.outputs[outDevice].channels[midiChannel].sendControlChange(35, 123);
            console.log("req Data on", midiChannel);
        }
    }
}

function su() {
    document.getElementById("firmware").innerHTML = Version;

    if (typeof (Storage) !== "undefined") {
        hasStorage = true;
    } else {
        hasStorage = false;
        errormsg("Storage unavailable");
    }
    setTimeout(devListener, 1200);
}

function storeChan() {
    if (hasStorage) {
        localStorage.setItem("midiCh", midiChannel);
    }
}

function handleCC(control, value) {
    console.log("recieved", midiChannel, control, value);
    if (control) {
        if (control == 35) {
            storeChan();
            switch (value) {
                case 124: // U(1)
                    //console.log("DONE---------------");
                    window.location.assign("u1.html");
                    break;
                case 125: // SU(2)

                    break;
                case 126: // SU(3)

                    break;
                default:
                    break;
            }
        }
    }
}

function setInDevice() {
    let inDevice = 0;
    if (opt == 0) {
        if (inMatch > -1) {
            inDevice = inMatch;
        }
    } else {
        inDevice = inSel.value;
    }
    console.log("setInDevice", inDevice);
    if (hasStorage) {
        localStorage.setItem("inputD", inDevice);
    }
    if (WebMidi.inputs.length > 0) {
        WebMidi.inputs[inDevice].removeListener();
        WebMidi.inputs[inDevice].addListener("midimessage", e => {
            if (e.message.channel && (e.message.channel != midiChannel)) {
                midiChannel = e.message.channel;
                midiDdVal = midiChannel;
                storeChan();
                console.log("set midiChannel to", midiChannel);

            }
            if (e.message.data[0] > 191) {
                // PC
            } else {
                handleCC(e.message.data[1], e.message.data[2]);
            }
        });
    }
}
function setOutDevice() {
    let outDevice = 0;
    if (opt == 0) {
        if (outMatch > -1) {
            outDevice = outMatch;
        }
    } else {
        outDevice = outSel.value;
    }
    console.log("setOutDevice", outDevice);
    if (hasStorage) {
        localStorage.setItem("outputD", outDevice);
    }
}
function devListener() {
    WebMidi.addListener("portschanged", e => {
        console.log("Devices changed");
        onEnabled();
    });
}
function toggle(tog) {
    let stopt = document.getElementById('stopt');
    opt = tog;
    if (tog) {
        stopt.style.height = stopt.scrollHeight + "px";
    } else {
        stopt.style.height = 0;
    }
}