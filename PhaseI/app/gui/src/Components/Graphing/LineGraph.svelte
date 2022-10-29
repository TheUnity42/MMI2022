<script>
	import { graphWidth, graphHeight } from '../../stores/GraphStore.js';
	export let xrng;
	export let yrng;

	// generate random y data and sequential x data
	const data = [];
	for (let i = 0; i < xrng; i++) {
		data.push({
			x: i,
			y: Math.sin(0.1*i) + 2//Math.random() * yrng
		});
	}

    // on load, make an svg path from the data
    let path = '';
    let xScale = 0;
    let yScale = 0;
    // on load
    $: {
        path = '';
        xScale = $graphWidth / xrng;
        yScale = $graphHeight / yrng;
        for (let i = 0; i < data.length; i++) {
            path += `${i == 0 ? 'M' : 'L'} ${data[i].x * xScale} ${$graphHeight - data[i].y * yScale} `;
        }
    }

</script>

<!-- Draw an SVG Path -->
<path d={path} stroke="#ffffff" fill="none" />
