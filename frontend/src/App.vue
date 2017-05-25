<template>
  <div id="app">
    <el-row :gutter="10">
      <el-col :xs="8" :sm="6" :md="4" :lg="3">
        <!-- Left column -->
        Left
      </el-col>
      <el-col :xs="4" :sm="6" :md="8" :lg="9">
        <!-- Right column -->
        <el-form ref="form" :model="lampState" label-width="20%">
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
      </el-col>
    </el-row>
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
      brightness : {
          get: function () {
              return this.lampState.presets[this.lampState.activePreset].brightness;
          },
          // setter
          set: function (newValue) {
              this.lampState.presets[this.lampState.activePreset].brightness = newValue;
          }
      },
      spread : {
          get: function () {
              return this.lampState.presets[this.lampState.activePreset].spread;
          },
          // setter
          set: function (newValue) {
              this.lampState.presets[this.lampState.activePreset].spread = newValue;
          }
      },
      fade : {
          get: function () {
              return this.lampState.presets[this.lampState.activePreset].fade;
          },
          // setter
          set: function (newValue) {
              this.lampState.presets[this.lampState.activePreset].fade = newValue;
          }
      },
  },
  methods: {

  }
}
</script>

<style>
body {
  font-family: Helvetica, sans-serif;
}
</style>
