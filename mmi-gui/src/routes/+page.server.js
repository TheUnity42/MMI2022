import { error } from '@sveltejs/kit';

/** @type {import('./$types').PageServerLoad} */
export async function load({ params }) {
	const post = {
		body: {
			title: 'Server Title?'
		}
	};

	console.log('Server Title: ', post);

	if (post) {
		return post;
	}

	throw error(404, 'Not found');
}
