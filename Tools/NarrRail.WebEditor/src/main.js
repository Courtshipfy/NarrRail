import { createApp } from "vue";
import App from "./App.vue";
import "./styles/editor.css";
import { installMaterialSymbolFallback } from "./utils/material-symbol-fallback.js";

const app = createApp(App);
app.mount("#app");
installMaterialSymbolFallback();
