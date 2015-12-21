    /* check MIN_ENTRY constraint here, move between groups if necessary */
    if(num_items[0] < MIN_ENTRIES)
    {
	/* group0 has too few members */
	unsigned diff = MIN_ENTRIES - num_items[0];
	for(i=0; i<diff; i++)
	{
	    GET_FIRSTITEM(destitem, split0);
	    destitem += (num_items[0]++)*ENTRYSZ;
	    GET_FIRSTITEM(item, split1);
	    item += (--num_items[1])*ENTRYSZ;
	    memcpy(destitem, item, ENTRYSZ);
	}
    }
    if(num_items[1] < MIN_ENTRIES)
    {
	/* group1 has too few members */
	unsigned diff = MIN_ENTRIES - num_items[1];
	for(i=0; i<diff; i++)
	{
	    GET_FIRSTITEM(destitem, split1);
	    destitem += (num_items[1]++)*ENTRYSZ;
	    GET_FIRSTITEM(item, split0);
	    item += (--num_items[0])*ENTRYSZ;
	    memcpy(destitem, item, ENTRYSZ);
	}
    }


/* linear pick seeds is annuled because Guttman did specify it clearly */
void
linear_pick_seeds(unsigned res[2],
		  void *items, unsigned num_items, Rectangle *mbr)
{
    int i;
    Rectangle er1, er2;
    long x_ul, x_lr;

    /* LPS1: find extreme rectangles along 2 dimensions */
    er1.x_ul = mbr->x_ul;
    er1.y_ul = mbr->y_ul;
    er1.x_lr = mbr->x_lr;
    er1.y_lr = mbr->y_lr;
    er2.x_ul = mbr->x_ul;
    er2.y_ul = mbr->y_ul;
    er2.x_lr = mbr->x_lr;
    er2.y_lr = mbr->y_lr;
    x_ul = mbr->x_ul;
    x_lr = mbr->x_lr;

    for(i=0; i<num_items; i++)
    {
	char *item = items + i*ENTRYSZ;
	Rectangle r;

	memcpy(&r.x_ul, item, sizeof(long));
	memcpy(&r.y_ul, item+sizeof(long), sizeof(long));
	memcpy(&r.x_lr, item+2*sizeof(long), sizeof(long));
	memcpy(&r.y_lr, item+3*sizeof(long), sizeof(long));
	if(cmp_lowside(r, er1) > 0)
	    er1 = r;
	if(cmp_highside(r, er2) < 0)
	    er2 = r;
	if(r.x_ul < x_ul)
	    x_ul = r.x_ul;
	if(r.x_lr > x_lr)
	    x_lr = r.x_lr;
    }

    if(debug)
    {
	printf("Extreme rectangles: [(%ld,%ld)(%ld,%ld) (%ld,%ld)(%ld,%ld)]\n",
	       er1.x_ul, er1.y_ul, er1.x_lr, er1.y_lr,
	       er2.x_ul, er2.y_ul, er2.x_lr, er2.y_lr);
	printf("width = %ld\n", x_lr - x_ul);
    }

    /* LPS2: Adjust for shape of the rectangle cluster */
    
    
    /* LPS3: Select the most extreme pair */
}

