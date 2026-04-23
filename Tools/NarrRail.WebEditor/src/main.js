import { createApp } from 'vue';
import App from './App.vue';
import './styles/editor.css';

console.log('Main.js loaded');

const app = createApp(App);
app.mount('#app');

console.log('App mounted');
