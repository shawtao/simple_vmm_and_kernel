SECTIONS
{      
        .payload32 0 : {
                guest32 = .;
                ../build/obj/main32.img.o
                guest32_end = .;
	}
}
