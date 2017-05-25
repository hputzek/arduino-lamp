<template>
    <div id="app" style="max-width: 320px; margin: 0 auto;">
        <div class="container">
            <h1>Lamp</h1>
            <el-form ref="form" :model="lampState">
                <el-form-item label="Preset">
                    <el-button-group>
                        <el-button type="primary">1</el-button>
                        <el-button type="primary">2</el-button>
                        <el-button type="primary">3</el-button>
                        <el-button type="primary">4</el-button>
                        <el-button type="primary">5</el-button>
                    </el-button-group>
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
                            brightness: 10,
                            spread: 0,
                            fade: false
                        }
                    }
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
        methods: {}
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
