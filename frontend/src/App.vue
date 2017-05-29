<template>
    <div id="app" style="max-width: 320px; margin: 0 auto;">
        <div class="container">
            <h1>Lamp</h1>
            <el-form ref="form" :model="lampState">
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
        </div>
    </div>
</template>

<script>
    export default {
        data () {
            return {
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
                state: function(val){
                    console.log(val);
                },

                fade: function (val) {
                  console.log(val);
                },

                 connect() {
                 console.log("Websocket connected to " + this.$socket.nsp);
                 },

                 disconnect() {
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
    body {
        font-family: Helvetica, sans-serif;
        background: radial-gradient(black 15%, transparent 16%) 0 0,
        radial-gradient(black 15%, transparent 16%) 8px 8px,
        radial-gradient(rgba(255, 255, 255, .1) 15%, transparent 20%) 0 1px,
        radial-gradient(rgba(255, 255, 255, .1) 15%, transparent 20%) 8px 9px;
        background-color: #282828;
        background-size: 16px 16px;
    }

    #app {
        background: #efefef;
    }
    .container {
        padding: 20px;
    }

    .el-form-item__label {
        display: block;
        float: none;
        text-align: left;
    }
</style>
