void
fibonacci(Monitor *mon, int s) {
	unsigned int i, n, nx, ny, nw, nh;
	Client *c;

	for(n = 0, c = nexttiled(mon->clients); c; c = nexttiled(c->next), n++);
	if(n == 0)
		return;
	
	nx = mon->wx + mon->gappx;
	ny = mon->wy + mon->gappx;
	nw = mon->ww - 2 * mon->gappx;
	nh = mon->wh - 2 * mon->gappx;
	
	for(i = 0, c = nexttiled(mon->clients); c; c = nexttiled(c->next)) {
		if((i % 2 && (nh - mon->gappx) / 2 > 2 * c->bw)
		   || (!(i % 2) && (nw - mon->gappx) / 2 > 2 * c->bw)) {
			if(i < n - 1) {
				if(i % 2)
					nh = (nh - mon->gappx) / 2;
				else
					nw = (nw - mon->gappx) / 2;
				if((i % 4) == 2 && !s)
					nx += nw + mon->gappx;
				else if((i % 4) == 3 && !s)
					ny += nh + mon->gappx;
			}
			if((i % 4) == 0) {
				if(s)
					ny += nh + mon->gappx;
				else
					ny -= nh + mon->gappx;
			}
			else if((i % 4) == 1)
				nx += nw + mon->gappx;
			else if((i % 4) == 2)
				ny += nh + mon->gappx;
			else if((i % 4) == 3) {
				if(s)
					nx += nw + mon->gappx;
				else
					nx -= nw + mon->gappx;
			}
			if(i == 0) {
				if(n != 1)
					nw = (mon->ww - 2*mon->gappx) * mon->mfact - mon->gappx;
				ny = mon->wy + mon->gappx;
			}
			else if(i == 1)
				nw = mon->ww - nw - 3*mon->gappx;
			i++;
		}
		resize(c, nx, ny, nw - 2 * c->bw, nh - 2 * c->bw, False);
	}
}

void
dwindle(Monitor *mon) {
	fibonacci(mon, 1);
}

void
spiral(Monitor *mon) {
	fibonacci(mon, 0);
}
