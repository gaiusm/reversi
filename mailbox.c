#define mailbox_c

#include "mailbox.h"

#define NO_MAILBOXES 100

static void *shared_memory = NULL;
static mailbox *freelist = NULL;  /* list of free mailboxes.  */
static sem_t *mailbox_mutex = NULL;
static int mailbox_count = 0;


/*
 *  initialise the data structures of mailbox.  Assign next to the
 *  mailbox next field.
 */

static mailbox *mailbox_config (mailbox *mbox, mailbox *next)
{
  mbox->in = 0;
  mbox->out = 0;
  mbox->next = next;
  mbox->item_available = multiprocessor_initSem (0);
  mbox->space_available = multiprocessor_initSem (MAX_MOVES);
  mbox->mutex = multiprocessor_initSem (1);
  return mbox;
}


/*
 *  init_memory - initialise the shared memory region once.
 *                It also initialises all mailboxes.
 */

static void init_memory (void)
{
  if (shared_memory == NULL)
    {
      mailbox *mbox;
      mailbox *next = NULL;
      int i;
      _M2_multiprocessor_init ();
      shared_memory = multiprocessor_initSharedMemory
	(NO_MAILBOXES * sizeof (mailbox));
      mbox = shared_memory;
      for (i = NO_MAILBOXES-1; i >= 0; i--)
	next = mailbox_config (&mbox[i], next);
      freelist = next;
      mailbox_mutex = multiprocessor_initSem (1);
    }
}


/*
 *  init - create a single mailbox which can contain a single triple.
 */

mailbox *mailbox_init (void)
{
  mailbox *mbox;

  multiprocessor_wait (mailbox_mutex);
  mailbox_count++;
  // printf ("alloc mailbox %d\n", mailbox_count);
  mbox = freelist;
  freelist = freelist->next;
  multiprocessor_signal (mailbox_mutex);
  return mbox;
}


/*
 *  kill - return the mailbox to the freelist.  No process must use this
 *         mailbox.
 */

mailbox *mailbox_kill (mailbox *mbox)
{
  multiprocessor_wait (mailbox_mutex);
  mbox->next = freelist;
  freelist = mbox;
  mailbox_count--;
  // printf ("free mailbox %d\n", mailbox_count);
  multiprocessor_signal (mailbox_mutex);
  return NULL;
}


/*
 *  send - send (result, move_no, move_square) to the mailbox mbox.
 */

void mailbox_send (mailbox *mbox, int result, int move_no, int move_square)
{
  multiprocessor_wait (mbox->space_available);
  multiprocessor_wait (mbox->mutex);
  mbox->data[mbox->in].result = result;
  mbox->data[mbox->in].move_no = move_no;
  mbox->data[mbox->in].move_square = move_square;
  mbox->in = (mbox->in + 1) % MAX_MOVES;
  multiprocessor_signal (mbox->mutex);
  multiprocessor_signal (mbox->item_available);
}


/*
 *  rec - receive (result, move_no, move_square) from the
 *        mailbox mbox.
 */

void mailbox_rec (mailbox *mbox,
		  int *result, int *move_no, int *move_square)
{
  multiprocessor_wait (mbox->item_available);
  multiprocessor_wait (mbox->mutex);
  *result = mbox->data[mbox->out].result;
  *move_no = mbox->data[mbox->out].move_no;
  *move_square = mbox->data[mbox->out].move_square;
  mbox->out = (mbox->out + 1) % MAX_MOVES;
  multiprocessor_signal (mbox->mutex);
  multiprocessor_signal (mbox->space_available);
}


/* Initialize the mailbox module.  */

void mailbox_initialization (void)
{
  init_memory ();
  if (freelist == NULL)
    {
      printf ("exhausted mailboxes\n");
      exit (1);
    }
}
