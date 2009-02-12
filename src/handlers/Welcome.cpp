
CLIENT_F_FUNC(Welcome)
{
	PacketBuilder reply;

	int id;

	switch (action)
	{
		case PACKET_REQUEST: // Selected a character

			reader.GetByte(); // Ordering byte

			reader.GetByte(); // ??
			id = reader.GetInt(); // Character ID

			reply.SetID(PACKET_WELCOME, PACKET_REPLY);

			reply.AddShort(1); // ??
			reply.AddChar(84); // ??
			reply.AddChar(51); // ??
			reply.AddInt(id); // Character ID
			reply.AddShort(84); // Map ID
			reply.AddChar(55); // ??
			reply.AddChar(165); // ??
			reply.AddChar(217); // ??
			reply.AddChar(55); // ??
			reply.AddChar(30); // ??
			reply.AddShort(7); // ??
			reply.AddChar(56); // ?? **
			reply.AddChar(65); // ?? **
			reply.AddChar(2); // ?? **
			reply.AddChar(200); // ?? **
			reply.AddChar(231); // ?? **
			reply.AddChar(1); // ?? **
			reply.AddChar(103); // ?? **
			reply.AddChar(219); // ?? **
			reply.AddChar(25); // ?? **
			reply.AddChar(122); // ?? **
			reply.AddChar(3); // ?? **
			reply.AddChar(1); // ?? **
			reply.AddChar(47); // ?? **
			reply.AddChar(177); // ?? **
			reply.AddChar(183); // ?? **
			reply.AddChar(45); // ?? **
			reply.AddShort(32); // ?? **
			reply.AddChar(154); // ?? **
			reply.AddChar(77); // ?? **
			reply.AddChar(177); // ?? **
			reply.AddChar(221); // ?? **
			reply.AddShort(8); // ?? **
			reply.AddBreakString("a"); // Name
			reply.AddByte(255); // ?? **
			reply.AddByte(255); // ?? **
			reply.AddByte(255); // ?? **
			reply.AddByte(255); // ?? **
			reply.AddChar(1); // ??
			reply.AddChar(31); // ?? **
			reply.AddChar(31); // ?? **
			reply.AddChar(31); // ?? **
			reply.AddChar(0); // ?? **
			reply.AddChar(3); // ??
			reply.AddChar(105); // ??
			reply.AddThree(14); // ??
			reply.AddChar(128); // ??
			reply.AddThree(1); // ??
			reply.AddShort(16); // ??
			reply.AddShort(16); // ??
			reply.AddShort(17); // ??
			reply.AddShort(17); // ??
			reply.AddShort(23); // ??
			reply.AddShort(9); // ??
			reply.AddShort(12); // ??
			reply.AddChar(241); // ?? **
			reply.AddChar(3); // ?? **
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(1); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(1); // ??
			reply.AddShort(100); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddChar(211); // ??
			reply.AddChar(1); // ?? **
			reply.AddChar(14); // ??
			reply.AddChar(1); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddShort(0); // ??
			reply.AddChar(0); // ?? **
			reply.AddShort(76); // ?? **
			reply.AddShort(4); // ?? **
			reply.AddChar(24); // ?? **
			reply.AddChar(24); // ?? **
			reply.AddShort(10); // ?? **
			reply.AddShort(10); // ?? **
			reply.AddShort(1); // ?? **
			reply.AddShort(1); // ?? **
			reply.AddChar(0); // ?? **
			reply.AddByte(255); // ?? **

			CLIENT_SEND(reply);

			break;

		case PACKET_MSG: // Welcome message after you login.

			reader.GetByte(); // Ordering byte

			reader.GetThree(); // ??
			id = reader.GetInt(); // Character ID

			reply.SetID(PACKET_WELCOME, PACKET_REPLY);

			reply.AddShort(2);
			reply.AddByte(255);
			reply.AddBreakString("Message 1");
			reply.AddByte(255);
			reply.AddBreakString("Message 2");
			reply.AddByte(255);
			reply.AddBreakString("Message 3");
			reply.AddByte(255);
			reply.AddBreakString("Message 4");
			reply.AddByte(255);
			reply.AddBreakString("Message 5");
			reply.AddByte(255);
			reply.AddByte(255);
			reply.AddByte(255);
			reply.AddByte(255);
			reply.AddByte(255);
			reply.AddChar(250);
			reply.AddChar(75);
			reply.AddShort(214);
			reply.AddInt(2);
			reply.AddShort(212);
			reply.AddInt(1);
			reply.AddChar(1);
			reply.AddShort(16);
			reply.AddInt(1);
			reply.AddShort(217);
			reply.AddInt(1);
			reply.AddShort(220);
			reply.AddInt(2);
			reply.AddShort(218);
			reply.AddInt(1);
			reply.AddShort(67);
			reply.AddInt(81);
			reply.AddShort(68);
			reply.AddInt(5);
			reply.AddShort(64);
			reply.AddInt(7);
			reply.AddShort(71);
			reply.AddInt(1);
			reply.AddShort(54);
			reply.AddInt(7);
			reply.AddShort(327);
			reply.AddInt(2);
			reply.AddShort(336);
			reply.AddInt(63390);
			reply.AddShort(12);
			reply.AddInt(28077);
			reply.AddShort(45);
			reply.AddInt(26621);
			reply.AddByte(255);
			reply.AddByte(255);
			reply.AddChar(2);
			reply.AddByte(255);
			reply.AddChar(102);
			reply.AddChar(96);
			reply.AddChar(114);
			reply.AddChar(115);
			reply.AddChar(96);
			reply.AddChar(115);
			reply.AddChar(104);
			reply.AddChar(110);
			reply.AddChar(109);
			reply.AddChar(114);
			reply.AddByte(255);
			reply.AddChar(245);
			reply.AddChar(47);
			reply.AddShort(154);
			reply.AddShort(30);
			reply.AddShort(6);
			reply.AddChar(3);
			reply.AddChar(5);
			reply.AddChar(66);
			reply.AddChar(79);
			reply.AddChar(77);
			reply.AddChar(4);
			reply.AddChar(0);
			reply.AddChar(12);
			reply.AddChar(8);
			reply.AddChar(2);
			reply.AddShort(24);
			reply.AddShort(19);
			reply.AddShort(19);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(31);
			reply.AddShort(0);
			reply.AddShort(15);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddChar(2);
			reply.AddChar(0);
			reply.AddByte(255);
			reply.AddChar(120);
			reply.AddChar(110);
			reply.AddChar(107);
			reply.AddChar(116);
			reply.AddChar(110);
			reply.AddChar(114);
			reply.AddChar(115);
			reply.AddChar(115);
			reply.AddChar(103);
			reply.AddChar(100);
			reply.AddChar(98);
			reply.AddChar(110);
			reply.AddChar(111);
			reply.AddChar(120);
			reply.AddByte(255);
			reply.AddChar(8);
			reply.AddChar(28);
			reply.AddShort(154);
			reply.AddShort(37);
			reply.AddShort(7);
			reply.AddChar(0);
			reply.AddChar(6);
			reply.AddChar(31);
			reply.AddChar(31);
			reply.AddChar(31);
			reply.AddChar(2);
			reply.AddChar(1);
			reply.AddByte(9);
			reply.AddByte(1);
			reply.AddByte(0);
			reply.AddShort(15);
			reply.AddShort(15);
			reply.AddShort(14);
			reply.AddShort(14);
			reply.AddShort(52);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddChar(0);
			reply.AddChar(0);
			reply.AddByte(255);
			reply.AddByte(255);
			CLIENT_SEND(reply);

			break;

		default:
			return false;
	}

	return true;
}
