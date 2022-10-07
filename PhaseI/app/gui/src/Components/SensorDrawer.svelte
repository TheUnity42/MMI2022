<script>
    import { sensorStore } from '../stores/SensorStore.js';
    import SensorModality from './SensorModality.svelte';

    let addSensor = () => {
        sensorStore.update((sensors) => {
            sensors.push("Sensor " + (sensors.length + 1));
            return sensors;
        });
    };    
</script>

<div class="drawer">
  <input id="sensor-drawer" type="checkbox" class="drawer-toggle" />
  <div class="drawer-content">
    <slot />    
  </div> 
  <div class="drawer-side">
    <label for="sensor-drawer" class="drawer-overlay"></label>
    <ul class="menu p-5 overflow-y-auto w-80 bg-base-100 text-base-content">
      <!-- Sidebar content here -->
      {#each $sensorStore as sensor}
        <li><label for="{sensor}-sensor-modality">{sensor}</label></li>
      {/each}
      <li class="absolute bottom-5"><button class="px-15" on:click={addSensor}>Add Sensor</button></li>
    </ul>
    {#each $sensorStore as sensor}
        <SensorModality name={sensor} />
    {/each}
  </div>
</div>