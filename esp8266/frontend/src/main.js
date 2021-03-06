'use strict'

import Vue from "vue";
import {Button, ButtonGroup, Form, FormItem, RadioButton, RadioGroup, Slider, Switch, Notification, Tabs, TabPane, Step, Steps, Input} from "element-ui";
// Vue.use(Icon);
// Vue.use(Row);
// Vue.use(Col);
// Vue.use(Upload);
// Vue.use(Progress);
// Vue.use(Spinner);
// Vue.use(Badge);
// Vue.use(Card);
// Vue.use(Rate);
Vue.use(Steps);
Vue.use(Step);
// Vue.use(Carousel);
// Vue.use(Scrollbar);
// Vue.use(CarouselItem);
// Vue.use(Collapse);
// Vue.use(CollapseItem);
// Vue.use(Cascader);
// Vue.use(ColorPicker);
// Vue.use(Notification);
//
// Vue.use(Loading.directive);
//
// Vue.prototype.$loading = Loading.service;
// Vue.prototype.$msgbox = MessageBox;
// Vue.prototype.$alert = MessageBox.alert;
// Vue.prototype.$confirm = MessageBox.confirm;
// Vue.prototype.$prompt = MessageBox.prompt;
Vue.prototype.$notify = Notification;
// Vue.prototype.$message = Message;
import "element-ui/lib/theme-default/index.css";
import App from "./App.vue";
import Loader from "./Loader.vue";
Vue.use(Loader);
Vue.component(Loader.name, Loader);

// Vue.use(Pagination);
// Vue.use(Dialog);
// Vue.use(Autocomplete);
// Vue.use(Dropdown);
// Vue.use(DropdownMenu);
// Vue.use(DropdownItem);
// Vue.use(Menu);
// Vue.use(Submenu);
// Vue.use(MenuItem);
// Vue.use(MenuItemGroup);
Vue.use(Input);
// Vue.use(InputNumber);
// Vue.use(Radio);
Vue.use(RadioGroup);
Vue.use(RadioButton);
// Vue.use(Checkbox);
// Vue.use(CheckboxGroup);
Vue.use(Switch);
// Vue.use(Select);
// Vue.use(Option);
// Vue.use(OptionGroup);
Vue.use(Button);
Vue.use(ButtonGroup);
// Vue.use(Table);
// Vue.use(TableColumn);
// Vue.use(DatePicker);
// Vue.use(TimeSelect);
// Vue.use(TimePicker);
// Vue.use(Popover);
// Vue.use(Tooltip);
// Vue.use(Breadcrumb);
// Vue.use(BreadcrumbItem);
Vue.use(Form);
Vue.use(FormItem);
Vue.use(Tabs);
Vue.use(TabPane);
// Vue.use(Tag);
// Vue.use(Tree);
// Vue.use(Alert);
Vue.use(Slider);
import VueRemote from 'vue-remote';
export const bus = new Vue();
Vue.use(
    VueRemote,
    {
        secure: false,
        host: "localhost",
        port: 8080,
        openHandler: function() {
            bus.$emit('websocketConnected', true);
        },
        closeHandler: function() {
            bus.$emit('websocketConnected', false);
        }
    }
);

new Vue({
    el: '#app',
    render: h => h(App)
});
