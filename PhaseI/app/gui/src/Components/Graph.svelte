<script>
	import { is_client } from "svelte/internal";

    // store width and height of this component
    let canvasWidth = 0;
    let canvasHeight = 0;
    let xSteps = 10;
    let ySteps = 10;

    const axesPadding = 2;
    const tickLength = 5;
</script>

<div class="w-full h-full" bind:clientWidth={canvasWidth} bind:clientHeight={canvasHeight}>
    <svg width="{canvasWidth}" height="{canvasHeight}">
        <!-- <rect x="0" y="0" width="100" height="100" fill="red" /> -->
        {#each {length: xSteps + 1} as _, i}
            <line x1="{100*i / xSteps}%" y1="0%" x2="{100*i / xSteps}%" y2="100%" stroke="black" stroke-opacity="0.15"/>
        {/each}
        {#each {length: ySteps + 1} as _, i}
            <line y1="{100*i / xSteps}%" x1="0%" y2="{100*i / xSteps}%" x2="100%" stroke="black" stroke-opacity="0.15"/>
        {/each}
    </svg>
    <!-- <p>{canvasWidth}px {canvasHeight}px</p> -->
</div>