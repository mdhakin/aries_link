Runtime Requirement

Aries-Vector currently runs on .NET 9.0.

The service launcher must call the correct runtime and DLL:

/root/.dotnet/dotnet /root/Aries-Vector/bin/Debug/net9.0/Aries-Vector.dll

If the service points to the net8.0 folder the system will launch an older build.

Systemd Service

/etc/systemd/system/aries-vector.service

[Unit]
Description=Aries Vector Motor Control Service
After=vectorcan.service
Requires=vectorcan.service

[Service]
Type=simple
ExecStart=/usr/local/bin/aries-vector-start
Restart=always
RestartSec=2
User=root

[Install]
WantedBy=multi-user.target

Launcher script:

/usr/local/bin/aries-vector-start

#!/bin/bash
exec /root/.dotnet/dotnet /root/Aries-Vector/bin/Debug/net9.0/Aries-Vector.dll

Make executable:

chmod +x /usr/local/bin/aries-vector-start
Note for Aries-Link README
Aries-Link Service

Aries-Link provides the radio command interface to the crawler and runs as a systemd service.

Binary location:

/root/aries_link/build/aries_link

Service file:

/etc/systemd/system/aries-link.service

[Unit]
Description=Aries Link Radio Bridge
After=aries-vector.service
Requires=aries-vector.service

[Service]
Type=simple
WorkingDirectory=/root/aries_link/build
ExecStart=/root/aries_link/build/aries_link
Restart=always
RestartSec=2
User=root

[Install]
WantedBy=multi-user.target

Enable and start:

systemctl daemon-reload
systemctl enable aries-link
systemctl start aries-link
Full Boot Chain

After configuration the crawler boots into a fully operational control stack:

Linux boot
   ↓
vectorcan.service        (CAN bus up)
   ↓
aries-vector.service     (motor control online)
   ↓
aries-link.service       (LoRa command interface online)

Once booted, the crawler can be controlled through Aries-Control without SSH access.

One small suggestion

Since you’re committing this milestone, you might want a commit message like:

Bring-up milestone:

• Added systemd services for vectorcan, aries-vector, and aries-link
• Aries-Vector now launches automatically at boot
• Aries-Link radio interface runs as a service
• Removed manual "go" requirement from Aries-Vector
• Documented runtime requirement (.NET 9.0)
• Documented service configuration

Crawler now boots to a fully operational state without SSH.

That commit message will be gold later when you look back.

And honestly Matt — this is a real robotics infrastructure milestone:

You now have a robot that:

boots
brings up CAN
launches motor control
launches command interface
accepts radio commands

That’s a proper autonomous system stack now.

Tomorrow when you pick it back up, the next really cool step will be expanding the command grammar and info board, because now the machine is stable enough to support it.
