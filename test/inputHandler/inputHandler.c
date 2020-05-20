#if defined (__cplusplus)
extern "C" {
#endif

#include "inputHandler.h"

#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <libinput.h>
#include <libudev.h>

#include <sys/ioctl.h>

#include "../../driver/CustomAssert.h"

struct libinput *li;
struct udev* _udev;

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

void initInputHandler()
{
	_udev = udev_new();
	udev_ref(_udev);
	li = libinput_udev_create_context(&interface, NULL, _udev); assert(li);
	uint32_t res = libinput_udev_assign_seat(li, "seat0"); assert(res == 0);

	libinput_dispatch(li);

	//configure devices
//		libinput_device_config_accel_set_speed()
//		libinput_device_config_accel_set_profile()
//		libinput_device_config_scroll_set_natural_scroll_enabled()

	struct libinput_event* event = 0;
	while ((event = libinput_get_event(li)) != 0)
	{
		uint32_t type = libinput_event_get_type(event);

		switch(type)
		{
		case LIBINPUT_EVENT_DEVICE_ADDED:
		{
			struct libinput_device* dev = libinput_event_get_device(event);
			printf("Device name: %s\n", libinput_device_get_name(dev));
			break;
		}
		};


		libinput_event_destroy(event);
		libinput_dispatch(li);
	}
}

void handleInput()
{
	assert(li);

	struct libinput_event* event = 0;
	while ((event = libinput_get_event(li)) != 0)
	{
		uint32_t type = libinput_event_get_type(event);

		switch(type)
		{
		case LIBINPUT_EVENT_DEVICE_ADDED:
		{
			struct libinput_device* dev = libinput_event_get_device(event);
			printf("Device name: %s\n", libinput_device_get_name(dev));
			break;
		}
		case LIBINPUT_EVENT_KEYBOARD_KEY:
		{
			uint32_t keyCode = libinput_event_keyboard_get_key(libinput_event_get_keyboard_event(event));
			enum libinput_key_state keyState = libinput_event_keyboard_get_key_state(libinput_event_get_keyboard_event(event));
			printf("Keypress keycode %u state %u\n", keyCode, keyState);
			break;
		}
		case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
		{
			double xCoord = libinput_event_pointer_get_absolute_x(libinput_event_get_pointer_event(event));
			double yCoord = libinput_event_pointer_get_absolute_y(libinput_event_get_pointer_event(event));
			printf("Pointer motion xcoord %lf ycoord %lf\n", xCoord, yCoord);
			break;
		}
		case LIBINPUT_EVENT_POINTER_BUTTON:
		{
			uint32_t button = libinput_event_pointer_get_button(libinput_event_get_pointer_event(event));
			enum libinput_button_state buttonState = libinput_event_pointer_get_button_state(libinput_event_get_pointer_event(event));
			printf("Pointer button button %u, state %u\n", button, buttonState);
			break;
		}
		case LIBINPUT_EVENT_POINTER_AXIS:
		{
			enum libinput_pointer_axis_source axisSource = libinput_event_pointer_get_axis_source(libinput_event_get_pointer_event(event));
			double axisValueVertical = libinput_event_pointer_get_axis_value(libinput_event_get_pointer_event(event), LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
			double axisValueHorizontal = libinput_event_pointer_get_axis_value(libinput_event_get_pointer_event(event), LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
			printf("Pointer axis  source %u, value vertical %lf, value horizontal %lf\n", axisSource, axisValueVertical, axisValueHorizontal);
			break;
		}
		};


		libinput_event_destroy(event);
		libinput_dispatch(li);
	}
}

void uninitInputHandler()
{
	libinput_unref(li);
	udev_unref(_udev);
}

#if defined (__cplusplus)
}
#endif
