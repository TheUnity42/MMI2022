<script>    
    import { graphWidth, graphHeight } from '../stores/GraphStore.js';
    // store width and height of this component

    let xSteps = 10;
    let ySteps = 10;

    const axesPadding = 2;
    const tickLength = 5;
</script>

<div class="w-full h-full outline shadow-md rounded-sm flex justify-center" bind:clientWidth={$graphWidth} bind:clientHeight={$graphHeight}>
    <svg width="{$graphWidth}" height="{$graphWidth}" class="m-auto">
        <!-- <rect x="0" y="0" width="100" height="100" fill="red" /> -->
        {#each {length: xSteps + 1} as _, i}
            <line x1="{100*i / xSteps}%" y1="0%" x2="{100*i / xSteps}%" y2="100%" stroke="black" stroke-opacity="0.15"/>
            <text x="{100*i / xSteps}%" y="100%" font-size="0.8em" fill="black">{i}</text>
        {/each}
        {#each {length: ySteps + 1} as _, i}
            <line y1="{100*i / xSteps}%" x1="0%" y2="{100*i / xSteps}%" x2="100%" stroke="black" stroke-opacity="0.15"/>
            <text y="{100*i / xSteps}%" x="0%" font-size="0.8em" fill="black">{ySteps-i}</text>
        {/each}
        <slot />
    </svg>
</div>