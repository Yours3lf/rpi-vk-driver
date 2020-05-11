#include <iostream>
#include <vector>
#include <algorithm>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <libinput.h>
#include <libudev.h>

static int open_restricted(const char *path, int flags, void *user_data)
{
		int fd = open(path, flags);
		return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data)
{
		close(fd);
}

const static struct libinput_interface interface = {
		.open_restricted = open_restricted,
		.close_restricted = close_restricted,
};


int main(void) {
		struct libinput *li;
		struct libinput_event *event;
		struct udev* udev;

		udev = udev_new();
		li = libinput_udev_create_context(&interface, NULL, udev);
		libinput_udev_assign_seat(li, "seat0");
		libinput_dispatch(li);

		while (true)
		{
			event = libinput_get_event(li);

			if(event)
			{
				uint32_t type = libinput_event_get_type(event);
				struct libinput_device* dev = libinput_event_get_device(event);

				// handle the event here
				std::cout << "Event type: " << type << std::endl;

				switch(type)
				{
				case LIBINPUT_EVENT_DEVICE_ADDED:
				{
					std::cout << "Device name: " << libinput_device_get_name(dev) << std::endl;
					break;
				}
				};


				libinput_event_destroy(event);
				libinput_dispatch(li);
			}
		}

		libinput_unref(li);

		return 0;
}
