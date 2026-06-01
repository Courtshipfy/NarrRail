import { createApp } from "vue";
import App from "./App.vue";
import IconGlyph from "./components/IconGlyph.vue";
import "./styles/editor.css";

const app = createApp(App);
app.component("IconGlyph", IconGlyph);
app.mount("#app");
