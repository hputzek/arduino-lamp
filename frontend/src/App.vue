<template>
    <div id="app" style="max-width: 480px; margin: 0 auto;">
        <div class="container">
            <div style="float:right" v-bind:class="{ 'led-blue': websocketConnected, 'led-red': !websocketConnected }"></div>
            <el-tabs name="tabs" value="first">
                <el-tab-pane label="Status" name="first">
                    <el-form ref="form" class="settings-form">
                        <el-form-item label="Preset">
                            <el-radio-group size="large" v-model="lampState.activePreset">
                                <el-radio-button label="1"></el-radio-button>
                                <el-radio-button label="2"></el-radio-button>
                                <el-radio-button label="3"></el-radio-button>
                                <el-radio-button label="4"></el-radio-button>
                                <el-radio-button label="5"></el-radio-button>
                            </el-radio-group>
                        </el-form-item>
                        <el-form-item label="Lamp state">
                            <el-switch
                                    v-model="lampState.state">
                            </el-switch>
                        </el-form-item>
                        <el-form-item label="Brightness">
                            <el-slider v-model="brightness"></el-slider>
                        </el-form-item>
                        <el-form-item label="Spread">
                            <el-slider v-model="spread"></el-slider>
                        </el-form-item>
                        <el-form-item label="Fading">
                            <el-switch
                                    v-model="fade">
                            </el-switch>
                        </el-form-item>
                    </el-form>
                </el-tab-pane>
                <el-tab-pane label="Settings" name="second">
                    <el-steps :space="115" :active="setup.step" :center="true">
                        <el-step title="SSID"></el-step>
                        <el-step title="Password"></el-step>
                        <el-step title="Connect"></el-step>
                    </el-steps>
                    <ul v-if="setup.step===1" class="wlans">
                        <template v-for="wifi in setup.wifis">
                            <li v-on:click="setWlan">{{wifi.ssid}}</li>
                        </template>
                    </ul>
                    <div v-if="setup.step===2">
                        <el-button @click="setup.step=1" style="width:100%">SSID: {{setup.wifi}}</el-button>
                        <el-form ref="form">
                            <el-form-item label="Password">
                                <el-input v-model="setup.wifiPassword"></el-input>
                            </el-form-item>
                            <el-button type="primary" @click="setup.step=3">Done</el-button>
                        </el-form>
                    </div>
                    <div v-if="setup.step===3">
                        <h3>Well done!</h3>
                        <i class="el-icon-circle-check" style="display: block; text-align: center; color: #42b983; font-size:120px;"></i>
                        <p>Your settings were saved. <br/>
                            I'm rebooting to the wifi of your choice.</p><p>If the connection fails, you can connect to the
                        default wifi again and give it another try.</p>
                    </div>
                </el-tab-pane>
            </el-tabs>
        </div>
    </div>
</template>

<script>
    export default {
        data () {
            return {
                websocketConnected: false,
                setup: {
                    step: 1,
                    wifi: '',
                    wifiPassword: '',
                    wifis: [
                        {ssid:'wlan1'},
                        {ssid:'wlan2'},
                        {ssid:'wlan3'}
                    ],
                },
                lampState: {
                    state: 0,
                    activePreset: 1,
                    presets: {
                        1: {
                            brightness: 0,
                            spread: 0,
                            fade: false
                        },
                        2: {
                            brightness: 0,
                            spread: 0,
                            fade: false
                        },
                        3: {
                            brightness: 0,
                            spread: 0,
                            fade: false
                        },
                        4: {
                            brightness: 0,
                            spread: 0,
                            fade: false
                        },
                        5: {
                            brightness: 0,
                            spread: 0,
                            fade: false
                        }
                    },
                }
            }
        },
        computed: {
            brightness: {
                get: function () {
                    return this.lampState.presets[this.lampState.activePreset].brightness;
                },
                // setter
                set: function (newValue) {
                    this.lampState.presets[this.lampState.activePreset].brightness = newValue;
                }
            },
            spread: {
                get: function () {
                    return this.lampState.presets[this.lampState.activePreset].spread;
                },
                // setter
                set: function (newValue) {
                    this.lampState.presets[this.lampState.activePreset].spread = newValue;
                }
            },
            fade: {
                get: function () {
                    return this.lampState.presets[this.lampState.activePreset].fade;
                },
                // setter
                set: function (newValue) {
                    this.lampState.presets[this.lampState.activePreset].fade = newValue;
                }
            },
        },
        watch: {
           'lampState.state': function(val) {
               this.$socket.emit('state', val);
            },
            'lampState.activePreset': function(val) {
                this.$socket.emit('activePreset', val);
            },
            brightness: function(val) {
                this.$socket.emit('brightness', val);
            },
            spread: function(val) {
                this.$socket.emit('spread', val);
            },
            fade: function(val) {
                this.$socket.emit('fade', val);
            }
        },
        methods: {
            setActiveState(e) {
                this.lampState.activePreset = e.target.innerText;
            },
            setWlan(e) {
                this.setup.step = 2;
                this.setup.wifi = e.target.innerText;
                console.log(e);
            }
        },
        socket: {
            // Prefix for event names
            // prefix: "/counter/",

            // If you set `namespace`, it will create a new socket connection to the namespace instead of `/`
            // namespace: "/counter",

            events: {

                // Similar as this.$socket.on("changed", (msg) => { ... });
                // If you set `prefix` to `/counter/`, the event name will be `/counter/changed`
                //
                setState: function(val){
                    let newState = JSON.parse(val);
                    if(newState !== null && typeof newState === 'object') {
                        this.lampState = Object.assign(this.lampState,newState);
                    }
                },

                fade: function (val) {
                  console.log(val);
                },

                 connect() {
                    this.websocketConnected = true;
                    console.log("Websocket connected to " + this.$socket.nsp);
                 },

                 disconnect() {
                     this.websocketConnected = false;
                     this.$notify.info({
                         title: 'Connection lost',
                         message: 'Oh no, Connection to lamp is gone :-('
                     });
                 console.log("Websocket disconnected from " + this.$socket.nsp);
                 },

                 error(err) {
                 console.error("Websocket error!", err);
                 }


            }
        }
    }
</script>

<style>

    #app {
        background: #efefef;
    }
    .container {
        padding: 15px;
    }

    .settings-form {
        padding: 10px;
    }

    .el-form-item__label {
        display: block;
        float: none;
        text-align: left;
    }

    .led-red {
        margin: 20px auto;
        width: 12px;
        height: 12px;
        background-color: #940;
        border-radius: 50%;
        box-shadow: #000 0 -1px 7px 1px, inset #600 0 -1px 9px, #F00 0 2px 12px;
    }

    .led-blue {
        margin: 20px auto;
        width: 12px;
        height: 12px;
        background-color: #4AB;
        border-radius: 50%;
        box-shadow: #000 0 -1px 7px 1px, inset #006 0 -1px 9px, #06F 0 2px 14px;
    }

    .wlans {
        list-style-type: none;
        margin: 0;
        padding: 0;
    }

    .wlans li {
        display: block;
        padding: 15px;
        margin:0;
        border-bottom: 1px solid #ccc;
    }
</style>
