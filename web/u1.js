let opt = [false, true];
let lst = true;
let v15 = ["Schottky", "Si", "Si asym", "Si+LED", "2Si+LED", "LED", "LED asym", "none"];
let preset = 42;
let psNum = 50; //total presets
let lasts = 0;
let nth = 0;
let hue = '#444';
let BA = new Uint8Array(2);

let inDevice = -1;
let outDevice = -1;

let midiChannel = 16;
let ulist = document.getElementById("psetb");
let midiDdVal = document.getElementById("midiCh").value;

WebMidi
    .enable()
    .then(onEnabled)
    .catch(err => console.log(err));

function onEnabled() {

    inDevice = localStorage.getItem("inputD");
    outDevice = localStorage.getItem("outputD");
    midiChannel = localStorage.getItem("midiCh");
    console.log("inD " + inDevice, "outD " + outDevice, "chan " + midiChannel);

    if (WebMidi.inputs.length > 0) {
        WebMidi.inputs[inDevice].removeListener();
        WebMidi.inputs[inDevice].addListener("midimessage", e => {
            if (e.message.channel && (e.message.channel != midiChannel)) {
                // midiChannel = e.message.channel;
                // midiDdVal = midiChannel;
                console.log("set midiChannel to", midiChannel);

            }
            if (e.message.data[0] > 191) {
                handlePC(e.message.data[1]);
            } else {
                handleCC(e.message.data[1], e.message.data[2]);
            }
        });
    }

    setTimeout(reqData, 50);
}
function reqData() {
    WebMidi.outputs[outDevice].channels[midiChannel].sendControlChange(35, 123);
    console.log("req Data on", midiChannel);
}

function su() {
    let obj;
    for (let i = 10; i < 36; i++) {
        if (obj = document.getElementById("s" + i)) {
            obj.addEventListener('input', sB);
        }
    }
    let listItems = ulist.getElementsByTagName("li");
    for (obj of listItems) {
        obj.addEventListener('click', function () {
            setPreset(this.innerText, true);
        });
    }
    too(0);
    window.addEventListener('resize', rsh);
}

Number.prototype.map = function (in_min, in_max, out_min, out_max) {
    return parseInt((this - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

function sendCC(cc, value) {
    let scaledVal = 0;
    value = +value;
    switch (+cc) {
        case 11:
            scaledVal = value.map(0, 20, 0, 127);
            break;
        case 12:
            scaledVal = value.map(-20, 0, 0, 127);
            break;
        case 13:
        case 14:
            scaledVal = value.map(-14, 14, 0, 127);
            break;
        case 15:
            scaledVal = value.map(0, 7, 0, 127);
            break;
        case 34:
            scaledVal = value.map(10, 100, 0, 127);
            break;
        case 35:
            scaledVal = value;
            break;
        default:
            break;
    }
    WebMidi.outputs[outDevice].channels[midiChannel].sendControlChange(cc, scaledVal);
}

function handleCC(control, value) {
    console.log("recieved", midiChannel, control, value);

    if (control) {
        let scaledVal = +value;
        switch (control) {
            case 11:    //gain
                scaledVal = value.map(0, 128, 0, 21);
                break;
            case 12:
                scaledVal = value.map(0, 128, -20, 1);
                break;
            case 13:
            case 14:
                scaledVal = value.map(0, 128, -14, 15);
                break;
            case 15:    //clipping
                scaledVal = value.map(0, 128, 0, 8);
                break;
            case 34:
                scaledVal = value.map(0, 128, 10, 101);
                break;
            default:
                break;
        }
        if (control < 35) {
            setS(control, scaledVal);
        } else {
            if (control == 35) {
                if (value < 17) {
                    document.getElementById("midiCh").value = value;
                } else {
                    if ((value > 19) && (value < 28)) {
                        setPhiMode(value - 20);
                    }
                }
            }
        }
    }
}

function handlePC(value) {
    value = parseInt(value);
    setPreset(value + 1, false);
}

function setPreset(pnumber, send) {
    preset = +pnumber;
    let listItems = ulist.getElementsByTagName("li");
    let txt;
    let first = 0;
    if (preset % 5) {
        first = (parseInt(preset / 5) * 5) + 1;
    } else {
        first = preset - 4;
    }

    //if (pnumber > psNum) {
    //        ulist.innerHTML = "bypassed";
    //}

    for (let i in listItems) {
        txt = first + +i;
        if (txt == preset) {
            listItems[i].className = "sel";
        } else {
            listItems[i].className = "";
        }
        if (txt > psNum) {
            txt = "";
        } else {
            if (txt < 10) {
                txt = "0" + txt;
            }
        }
        listItems[i].innerText = txt;
    }
    if (send) {
        WebMidi.outputs[outDevice].channels[midiChannel].sendProgramChange(preset - 1);
        console.log("sent PC", (preset - 1));
        sendCC(35, 123);
    } else {
        console.log("PC from pedal", (preset - 1));
    }
}
function sB() {
    let id = this.id.slice(1);
    let v = this.value;
    setVal(id, v, true);
}
function setS(id, v) {
    document.getElementById('s' + id.toString()).value = v;
    setVal(id, v, false);
}
function setVal(id, v, send) {
    if (send) {
        sendCC(+id, v);
    }
    switch (+id) {
        case 11:
        case 34:
            break;
        case 15:
            v = v15[v];
            break;
        default:
            v += " dB";
            break;
    }
    document.getElementById('v' + id).innerHTML = v;
}

function too(t) {
    let hhs = document.getElementById('hh' + t);
    let tos = document.getElementById('to' + t);
    if (opt[t]) {
        hhs.style.height = hhs.scrollHeight + "px";
        tos.innerHTML = '&xutri;';
    } else {
        hhs.style.height = 0;
        tos.innerHTML = '&xdtri;';
    }
    opt[t] = !opt[t];
}

function ch(doq) {
    if (doq.validity.valid) {
        doq.previousElementSibling.disabled = false;
    } else {
        doq.previousElementSibling.disabled = true;
    }
}
function butt(num) {
    switch (num) {
        case 0: //+
            if (preset < psNum) {
                preset++;
            } else {
                preset = 1;
            }
            break;
        case 1: //-
            if (preset > 1) {
                preset--;
            } else {
                preset = psNum;
            }
            break;
        case 2: //+5
            if (preset <= (psNum - 5)) {
                preset = preset + 5;
            } else {
                if (preset % 5) {
                    preset = preset % 5;
                } else {
                    preset = 5;
                }
            }
            break;
        case 3: //-5
            if (preset > 5) {
                preset = preset - 5;
            } else {
                if (preset % 5) {
                    let pt = (parseInt(psNum / 5) * 5) + preset;
                    if (pt > psNum) {
                        preset = (parseInt(psNum / 5) * 5) - (5 % preset);
                    } else {
                        preset = pt;
                    }
                } else {
                    preset = (parseInt(psNum / 5) * 5);
                }
            }
            break;
        default:
            break;
    }
    setPreset(preset, true);
}
function cp() {
    let cpb = document.getElementById('cp');
    if (opt[1]) {
        cpb.innerHTML = "PASTE"
        sendCC(35, 121);
    } else {
        cpb.innerHTML = "COPY"
        sendCC(35, 122);
    }
    opt[1] = !opt[1];
}
function sMidiChan(val) {
    if ((val > 0) && (val < 17)) {
        sendCC(35, val);
    }
}
function rsh() {
    if (!opt[0]) {
        let h = document.getElementById('hh0');
        h.style.height = "auto";
        h.style.height = h.scrollHeight + "px";
    }
}
function setPhiMode(phi) {
    let phiDiv = Math.min(phi, 2);
    for (let i = 0; i <= 2; i++) {
        let ph = document.getElementById('phi' + i);
        if (i == phiDiv) {
            ph.style.border = "solid 2px #888";
        } else {
            ph.style.border = "solid 2px #181818";
        }
    }
    if (phi > 2) {
        let pfunc = document.getElementById('phiFunc');
        pfunc.value = phi;
    }
}
function phiSel(phi) {
    phi = +phi;
    setPhiMode(phi);
    sendCC(35, (phi + 20));
}